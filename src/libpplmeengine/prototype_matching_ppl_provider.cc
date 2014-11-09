/**
 *  @file
 *  @brief   Implementation for pplme::engine::PrototypeMatchingPplProvider.
 *  @author  j.ho
 */


#include "prototype_matching_ppl_provider.h"
#include <math.h>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <boost/math/constants/constants.hpp>
#include <boost/scoped_ptr.hpp>
#include <glog/logging.h>


using pplme::core::GeoPosition;
using pplme::core::Person;


namespace {


// WGS84 [<http://en.wikipedia.org/wiki/World_Geodetic_System>].
float const kRadiusOfEarth = 6378.137;


int const kMinLatitudeDegrees = -90;
int const kMaxLatitudeDegrees = 90;
int const kMinLongitudeDegrees = -180;
int const kMaxLongitudeDegrees = 180;


size_t CalculateSizeForPplGrid(int resolution) {
  // Something of an arbitrary limit, but 1000 would take us to needing a
  // type larger than 32 bits to express all the grid position combos.
  CHECK(resolution <= 100) << "Too granular, son!";

  // + 2 because the ranges are inclusive.
  size_t num_quantized_latitude_values =
      ((kMaxLatitudeDegrees - kMinLatitudeDegrees) * resolution) + 2;
  size_t num_quantized_longitude_values =
      ((kMaxLongitudeDegrees - kMinLongitudeDegrees) * resolution) + 2;

  return num_quantized_latitude_values * num_quantized_longitude_values;
}


float ApproxDistance(GeoPosition::DecimalLatitude value1,
                     GeoPosition::DecimalLatitude value2) {
  // The value of 110.0 is just smaller than the range of possible values for
  // Terra's latitude.  This is by design---since accuracy isn't paramount we'd
  // rather be accidentally inclusive than accidentally exclusive.
  return abs((value2.value() - value1.value()) * 110.0);
}


float ApproxDistance(GeoPosition::DecimalLatitude latitude,
                     GeoPosition::DecimalLongitude value1,
                     GeoPosition::DecimalLongitude value2) {
  // Thank you, <http://en.wikipedia.org/wiki/Longitude#Length_of_a_degree_of_longitude>.
  using boost::math::constants::pi;
  auto result = cos(latitude.value() * pi<float>() / 180);
  result *= pi<float>() / 180;
  result *= kRadiusOfEarth;
  result *= (value2.value() - value1.value());
  return abs(result);
}


float ApproxDistance(GeoPosition position1, GeoPosition position2) {
  // This doesn't properly calculate the distance due it effectively assuming
  // a single latitude (to make the math easier).  This should be good enough
  // for pplMe; it's only going to start going really wrong as locations near
  // one of the Poles.
  auto longitudinally = ApproxDistance(
      position1.latitude(), position1.longitude(), position2.longitude());
  auto latitudinally = ApproxDistance(
      position1.latitude(), position2.latitude());
  return
      sqrt((longitudinally * longitudinally) + (latitudinally * latitudinally));
}


class NoddyWorkerPool {
 public:
  NoddyWorkerPool() {
    for (unsigned n = 0; n < std::thread::hardware_concurrency(); ++n)
      workers_.emplace_back([this]() { StartWorking(); });
  }


  ~NoddyWorkerPool() {
    /* lock block */ {
      std::unique_lock<std::mutex> lock(work_mutex_);
      die_ = true;
      work_or_die_.notify_all();
    }
    for (auto& worker : workers_)
      worker.join();
  }


  void QueueWorklette(std::function<void()> worklette) {
    std::unique_lock<std::mutex> lock(work_mutex_);
    work_.push_back(worklette);
    work_or_die_.notify_all();
  }
  
 private:
  std::vector<std::thread> workers_;
  std::mutex work_mutex_;
  std::condition_variable work_or_die_;
  std::deque<std::function<void()>> work_;
  bool die_{false};


  void StartWorking() {
    for (;;) {
      std::function<void()> worklette;

      /* lock block */ {
        std::unique_lock<std::mutex> lock(work_mutex_);
        if (die_)
          break;
        work_or_die_.wait(lock);

        if (die_)
          break;
 
        if (!work_.empty()) {
          worklette = work_.front();
          work_.pop_front();
        }
      }

      // Don't want to be holding locks while doing work!
      if (worklette)
        worklette();
    }
  }
};


}  // namespace


namespace pplme {
namespace engine {


class PrototypeMatchingPplProvider::Impl {
 public:
  Impl(int resolution,
       int max_distance,
       int max_age_difference,
       std::function<boost::gregorian::date()> date_provider) :
      resolution_{resolution},
      date_provider_{date_provider},
      max_distance_{max_distance},
      max_age_difference_{max_age_difference},
      ppl_{CalculateSizeForPplGrid(resolution)} {}


  ~Impl() {
    workers_.reset();
  }

  
  void Start() {
    workers_.reset(new NoddyWorkerPool{});
  }
  

  void AddPerson(std::unique_ptr<Person> person) {
    // We assume / don't-care if we've already seen a Person with the same id.
    ppl_[GetPplIndex(person->location_of_home())].push_back(std::move(person));
  }


  std::vector<Person>
  FindMatchingPpl(core::PplMatchingParameters const& parameters) const
  {
    std::vector<Person> ppl;

    // Start off by just considering the cell that the user conceptually
    // resides in.
    auto& grid_pos_ppl = ppl_[GetPplIndex(parameters.location_of_user())];
    FindMatchingPpl(parameters, grid_pos_ppl, &ppl);

    // This is how we keep track of async finds and rendezvous with them.
    struct FindPplOrchestration {
      std::mutex mutex;
      std::condition_variable condvar;
      std::vector<Person>* ppl;
      int pending_ops;
    };
    auto orchestration = std::make_shared<FindPplOrchestration>();
    orchestration->ppl = &ppl;
    
    // Then inspect the "nearby" cells.
    auto nearby_cells = GetNearbyCells(parameters.location_of_user());
    for (auto cell : nearby_cells) {
      // resultslette is just for this one op
      auto resultslette = std::make_shared<std::vector<Person>>();
      auto findppl_worklette =
          [this, parameters, cell, orchestration, resultslette]() {
        /* lock block */ {
          std::unique_lock<std::mutex> lock(orchestration->mutex);
          if (!orchestration->ppl) {
            --orchestration->pending_ops;
            orchestration->condvar.notify_one();
            return;
          }
        }

        // Yup, this is done asynchronously.  And it got me realizing that I
        // think I was a bit of a dumbass for putting the person's name in the
        // Person class; or, at least, not having a different class for storing
        // in the grid.  That way, you could have names (and whatever other
        // metadata) fetched asynchronously from some other datastore (RDBMS
        // or whatever) while the core engine busied itself smashing through
        // the problem of actually finding matches (the id is key, right?).
        // Oh and I also just realized that I could probably save quite a bit
        // of memory given the fact that name /is/ a member of Person by having
        // some sort of compression based on a rop of name atoms or somesuch.
        // Oh well, it's too late now, eh?  &:/
        FindMatchingPpl(
            parameters, ppl_[GetPplIndex(cell)], resultslette.get());

        /* lock block */ {
          std::unique_lock<std::mutex> lock(orchestration->mutex);
          if (orchestration->ppl) {
            for (auto person : *resultslette)
              orchestration->ppl->push_back(person);
          }
          --orchestration->pending_ops;
          orchestration->condvar.notify_one();
        }
      };

      /* lock bock */ {
        std::unique_lock<std::mutex> lock(orchestration->mutex);
        ++orchestration->pending_ops;
      }

      workers_->QueueWorklette(findppl_worklette);
    }

    /* lock block */ {
      std::unique_lock<std::mutex> lock(orchestration->mutex);
      orchestration->condvar.wait_for(
          lock,
          // Spec says to return within 1 second.  This is where that magic
          // happens.
          std::chrono::seconds(1),
          [orchestration]() { return orchestration->pending_ops == 0; });

      // Make sure that any unfinished work doesn't smash us up.
      orchestration->ppl = nullptr;
    }
    
    
    return ppl;
  }


 private:
  using Latitude = GeoPosition::DecimalLatitude;
  using Longitude = GeoPosition::DecimalLongitude;
  
  using PplCell = std::vector<std::unique_ptr<core::Person>>;
  using PplGrid = std::vector<PplCell>;
  
  int resolution_;
  std::function<boost::gregorian::date()> date_provider_;
  int max_distance_;
  int max_age_difference_;
  PplGrid ppl_;
  boost::scoped_ptr<NoddyWorkerPool> workers_;


  PplGrid::size_type GetLatitudeIndex(Latitude latitude) const {
    int latitude_index = latitude.value() + 90;
    CHECK(latitude_index >= 0 && latitude_index <= 180);
    return latitude_index * resolution_;
  }

  
  PplGrid::size_type GetLongitudeIndex(Longitude longitude) const {
    int longitude_index = longitude.value() + 180;
    CHECK(longitude_index >= 0 && longitude_index <= 360);
    return longitude_index * resolution_;
  }


  struct CellLocator {
    PplGrid::size_type latitude_index;
    PplGrid::size_type longitude_index;
  };


  CellLocator ToCellLocator(GeoPosition geopos) const {
    return CellLocator{
        GetLatitudeIndex(geopos.latitude()),
        GetLongitudeIndex(geopos.longitude())};
  }

  
  PplGrid::size_type GetPplIndex(CellLocator cell) const {
    return cell.latitude_index * 360 + cell.longitude_index;
  }

  
  PplGrid::size_type GetPplIndex(GeoPosition geopos) const {
    return GetPplIndex(ToCellLocator(geopos));
  }


  CellLocator TickNorth(CellLocator locator) const {
    // Don't need to handle wrap-around here.  IsTooFarNorth polices for us.
    return CellLocator{locator.latitude_index + 1, locator.longitude_index};
  }

  
  CellLocator TickSouth(CellLocator locator) const {
    // Don't need to handle wrap-around here.  IsTooFarSouth polices for us.
    return CellLocator{locator.latitude_index - 1, locator.longitude_index};
  }

  
  CellLocator TickEast(CellLocator locator) const {
    // Handle wrap-around.
    return CellLocator{
      locator.latitude_index,
      locator.longitude_index >= static_cast<unsigned>(360 * resolution_) ?
          0 : locator.longitude_index + 1};
  }

  
  CellLocator TickWest(CellLocator locator) const {
    // Handle wrap-around.
    return CellLocator{
      locator.latitude_index,
      locator.longitude_index == 0 ?
          360 * resolution_ - 1 : locator.longitude_index - 1};
  }


  bool IsTooFarNorth(GeoPosition origin, CellLocator cell) const {
    // Don't go past Pole.
    if (cell.latitude_index >= static_cast<unsigned>(180 * resolution_))
      return true;
    auto cell_southmost_latitude =
        (Latitude::ValueType(cell.latitude_index) / resolution_) - 90;
    return ApproxDistance(
        origin.latitude(),
        Latitude{cell_southmost_latitude}) > max_distance_;
  }

  
  bool IsTooFarSouth(GeoPosition origin, CellLocator cell) const {
    // Don't go past Pole.
    if (cell.latitude_index == 0)
      return true;
    auto cell_northmost_latitude =
        ((Latitude::ValueType(cell.latitude_index) + 1) / resolution_) - 90;
    return ApproxDistance(
        origin.latitude(),
        Latitude{cell_northmost_latitude}) > max_distance_;
  }


  bool IsTooFarFromOrigin(GeoPosition origin, CellLocator cell) const {
    // We travel at most 1/4 of the circumference (at whatever latitude).
    auto origin_cell = ToCellLocator(origin);
    return origin_cell.longitude_index - cell.longitude_index
        >= static_cast<unsigned>(90 * resolution_)
        && cell.longitude_index - origin_cell.longitude_index
        >= static_cast<unsigned>(90 * resolution_);
  }
  

  bool IsTooFarEast(GeoPosition origin, CellLocator cell) const {
    // At the Poles, we can basically end up spinning around the globe
    // because the distances become so small.  Make sure we have an out.
    if (IsTooFarFromOrigin(origin, cell))
      return true;
    auto cell_westmost_longitude =
        (Longitude::ValueType(cell.longitude_index) / resolution_) - 180;
    return ApproxDistance(
        origin.latitude(),
        origin.longitude(),
        Longitude{cell_westmost_longitude}) > max_distance_;
  }


  bool IsTooFarWest(GeoPosition origin, CellLocator cell) const {
    // At the Poles, we can basically end up spinning around the globe
    // because the distances become so small.  Make sure we have an out.
    if (IsTooFarFromOrigin(origin, cell))
      return true;
    auto cell_eastmost_longitude =
        ((Longitude::ValueType(cell.longitude_index) + 1) / resolution_) - 180;
    return ApproxDistance(
        origin.latitude(),
        origin.longitude(),
        Longitude{cell_eastmost_longitude}) > max_distance_;
  }


  void GetNearbyCellsSameLatitude(
      GeoPosition origin,
      CellLocator current_position,
      std::vector<CellLocator>* cells) const {
    for (auto cell = TickEast(current_position);
         !IsTooFarEast(origin, cell);
         cell = TickEast(cell)) {
      cells->push_back(cell);
    }
    for (auto cell = TickWest(current_position);
         !IsTooFarWest(origin, cell);
         cell = TickWest(cell)) {
      cells->push_back(cell);
    }
  }

  
  std::vector<CellLocator> GetNearbyCells(GeoPosition geopos) const {
    std::vector<CellLocator> cells;

    // Initially I went with iterating along the longitudinal axis first, but
    // at the Oasis Dartboard this evening/morning, I realized that iterating
    // first via latitude was more correct.  Doh.
    CellLocator const bullseye = ToCellLocator(geopos);
    GetNearbyCellsSameLatitude(geopos, bullseye, &cells);
    for (auto cell = TickNorth(bullseye);
         !IsTooFarNorth(geopos, cell);
         cell = TickNorth(cell)) {
        cells.push_back(cell);
      GetNearbyCellsSameLatitude(geopos, cell, &cells);
    }
    for (auto cell = TickSouth(bullseye);
         !IsTooFarSouth(geopos, cell);
         cell = TickSouth(cell)) {
      GetNearbyCellsSameLatitude(geopos, cell, &cells);
    }

    return cells;
  }


  void FindMatchingPpl(
      core::PplMatchingParameters const& parameters,
      std::vector<std::unique_ptr<Person>> const& grid_pos_ppl,
      std::vector<Person>* ppl) const {
    auto const today = date_provider_();

    for (auto const& person : grid_pos_ppl) {
      auto const earliest = today - boost::gregorian::years(
          parameters.age_of_user() + max_age_difference_);
      auto const latest = today - boost::gregorian::years(
          parameters.age_of_user() - max_age_difference_);
      // If they're in the same "grid position", then the Person is assumed to
      // be nearby, so the only other thing to consider is their age.
      if (   person->date_of_birth() >= earliest
          && person->date_of_birth() <= latest)
      {
        if (ApproxDistance(parameters.location_of_user(),
                           person->location_of_home()) <= max_distance_) {
          ppl->push_back(*person);
        }
      }
    }
  }
};


PrototypeMatchingPplProvider::PrototypeMatchingPplProvider(
    int resolution,
    int max_distance,
    int max_age_difference,
    std::function<boost::gregorian::date()> date_provider) :
    impl_{new Impl{
        resolution, max_distance, max_age_difference, date_provider}} {
  // Some of the approximations will start to go a little wonky as the maximum
  // distance gets big relative to the radius of Terra.
  CHECK(max_distance > 0 && max_distance < 1000);
  CHECK(max_age_difference >= 0);
  CHECK(date_provider);
}


PrototypeMatchingPplProvider::~PrototypeMatchingPplProvider() noexcept(true) =
  default;


void PrototypeMatchingPplProvider::Start() {
  impl_->Start();
}


void PrototypeMatchingPplProvider::AddPerson(
    std::unique_ptr<core::Person> person) {
  impl_->AddPerson(std::move(person));
}

  
std::vector<core::Person>
PrototypeMatchingPplProvider::FindMatchingPpl(
    core::PplMatchingParameters const& parameters) const {
  return impl_->FindMatchingPpl(parameters);
}


}  // namespace engine
}  // namespace pplme
