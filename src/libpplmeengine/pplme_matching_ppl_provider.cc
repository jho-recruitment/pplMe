/**
 *  @file
 *  @brief   Implementation for pplme::engine::PplmeMatchingPplProvider.
 *  @author  j.ho
 */


#include "pplme_matching_ppl_provider.h"
#include <math.h>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <set>
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
      ((2 + kMaxLatitudeDegrees - kMinLatitudeDegrees) * resolution);
  size_t num_quantized_longitude_values =
      ((2 + kMaxLongitudeDegrees - kMinLongitudeDegrees) * resolution);

  return num_quantized_latitude_values * num_quantized_longitude_values;
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
        else if (work_.empty())
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


class PplmeMatchingPplProvider::Impl {
 public:
  Impl(int resolution,
       int max_age_difference,
       int max_ppl,
       boost::optional<int> per_find_concurrency,
       std::function<boost::gregorian::date()> date_provider) :
      resolution_{resolution},
      date_provider_{date_provider},
      max_age_difference_{max_age_difference},
      max_ppl_{boost::numeric_cast<unsigned int>(max_ppl)},
      per_find_concurrency_{
          per_find_concurrency ?
              *per_find_concurrency : std::thread::hardware_concurrency()},
      ppl_{CalculateSizeForPplGrid(resolution)} {
    CHECK(max_age_difference >= 0);
    CHECK(max_ppl_ > 0);
    CHECK(!per_find_concurrency || per_find_concurrency > 0);
    CHECK(date_provider);
        
    workers_.reset(new NoddyWorkerPool{});
  }

  
  ~Impl() {
    workers_.reset();
  }
  

  void AddPerson(std::unique_ptr<Person> person) {
    // We assume / don't-care if we've already seen a Person with the same id.
    auto& cell = ppl_[GetPplIndex(person->location_of_home())];
    auto insertion_pos = std::upper_bound(
        begin(cell), end(cell), person,
        [](std::unique_ptr<Person> const& lhs,
           std::unique_ptr<Person> const& rhs) {
          return lhs->date_of_birth() < rhs->date_of_birth();
        });
    cell.insert(insertion_pos, std::move(person));
  }


  std::vector<Person>
  FindMatchingPpl(core::PplMatchingParameters const& parameters) const
  {
    FindContext context;
    context.parameters = &parameters;
    
    Sqiral(ToCellLocator(parameters.location_of_user()),
           [this, &context](CellLocator cell) {
             return TryFindPpl(cell, &context);
           });

    std::unique_lock<std::mutex> lock(context.mutex);
    context.condvar.wait(
        lock,
        [&context]() {
          return context.pending_cells.empty();
        });

    // Because we're likely multi-threaded, this list may not necessarily be
    // in order of distance.  If we were going for accuracy, we could sort
    // based on the haversine formula; however, this should be good enough for
    // pplMe purposes.
    if (context.ppl.size() > max_ppl_)
      context.ppl.resize(max_ppl_);

    return context.ppl;
  }


 private:
  using Latitude = GeoPosition::DecimalLatitude;
  using Longitude = GeoPosition::DecimalLongitude;
  
  using PplCell = std::vector<std::unique_ptr<core::Person>>;
  /** @note  This is sorted in date-of-birth order. */
  using PplGrid = std::vector<PplCell>;

  int resolution_;
  std::function<boost::gregorian::date()> date_provider_;
  int max_age_difference_;
  /* Having this unsigned is slightly against the Google C++ Style Guide;
     however, it means we don't have to cast everywhere to avoid warnings
     about signed/unsigned comparisons and all that guff. */
  unsigned int max_ppl_;
  unsigned int per_find_concurrency_;
  /** @note  This is sorted in date-of-birth order. */
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
    bool operator<(CellLocator rhs) const {
      if (latitude_index == rhs.latitude_index)
        return longitude_index < rhs.longitude_index;
      else
        return latitude_index < rhs.latitude_index;
    }
  };

  
  CellLocator ToCellLocator(GeoPosition geopos) const {
    return CellLocator{
        GetLatitudeIndex(geopos.latitude()),
        GetLongitudeIndex(geopos.longitude())};
  }

  
  PplGrid::size_type GetPplIndex(CellLocator cell) const {
    return cell.latitude_index * 361 * resolution_ + cell.longitude_index;
  }

  
  PplGrid::size_type GetPplIndex(GeoPosition geopos) const {
    return GetPplIndex(ToCellLocator(geopos));
  }

  
  enum class CheckOffsetsResult {
    Valid,
    Terminal,
    Invalid,
  };


  CheckOffsetsResult CheckOffsets(CellLocator origin,
                                  int lat_off,
                                  int long_off) const {
    auto result = CheckOffsetsResult::Invalid;
    
    int max_long_offset = 180 * resolution_;
    int max_lat_index = 181 * resolution_;
    int lat_index = boost::numeric_cast<int>(origin.latitude_index) + lat_off;
    
    if (long_off <= max_long_offset && long_off >= -max_long_offset) {
      if (lat_index >= 0 && lat_index <= max_lat_index) {
        if ((long_off == max_long_offset || long_off == -max_long_offset)
            && (lat_index == 0 || lat_index == max_lat_index))
          result = CheckOffsetsResult::Terminal;
        else
          result = CheckOffsetsResult::Valid;
      }
    }
    
    return result;
  }

  
  bool Sqiral(CellLocator origin, std::function<bool (CellLocator)> fun) const {
    bool we_done_here = false;

    int corner_count = 0;
    
    auto do_cell =
        [origin, fun, this, &corner_count](int lat_off, int long_off) {
      auto check_offsets_result = CheckOffsets(origin, lat_off, long_off);

      if (check_offsets_result == CheckOffsetsResult::Terminal)
        ++corner_count;

      if (check_offsets_result == CheckOffsetsResult::Terminal ||
          check_offsets_result == CheckOffsetsResult::Valid) {
        int long_index =
            boost::numeric_cast<int>(origin.longitude_index) + long_off;
        if (long_index > 0)
          long_index %= 361 * resolution_;
        else if (long_index != 0)
          long_index = ((361 * resolution_) + long_index) % (361 * resolution_);

        CellLocator cell{
            static_cast<unsigned int>(
                boost::numeric_cast<int>(origin.latitude_index) + lat_off),
            static_cast<unsigned int>(long_index)};
        return fun(cell);
      }
      return false;
    };

    we_done_here = do_cell(0, 0);

    for (int max_offset = 1; corner_count != 4 && !we_done_here; ++max_offset) {
      int north_offset = max_offset;
      int east_offset = 0;
      // N -> E
      for (; !we_done_here && north_offset > 0;
           ++east_offset, --north_offset) {
        we_done_here = do_cell(north_offset, east_offset);
      }
      // E -> S
      int south_offset = 0;
      east_offset = max_offset;
      for (; !we_done_here && east_offset > 0;
           ++south_offset, --east_offset) {
        we_done_here = do_cell(-south_offset, east_offset);
      }
      // S -> W
      int west_offset = 0;
      south_offset = max_offset;
      for (; !we_done_here && south_offset > 0;
           ++west_offset, --south_offset) {
        we_done_here = do_cell(-south_offset, -west_offset);
      }
      // W -> N
      north_offset = 0;
      west_offset = max_offset;
      for (; !we_done_here && west_offset > 0;
           ++north_offset, --west_offset) {
        we_done_here = do_cell(north_offset, -west_offset);
      }
    }

    return we_done_here;
  }


  struct FindContext {
    core::PplMatchingParameters const* parameters;
    std::vector<Person> ppl;
    std::set<CellLocator> pending_cells;
    std::mutex mutex;
    std::condition_variable condvar;
    bool we_done_here = false;
  };


  bool TryFindPpl(CellLocator cell, FindContext* context) const {
    CHECK_NOTNULL(context);
    CHECK_NOTNULL(context->parameters);
    
    std::unique_lock<std::mutex> lock(context->mutex);
    context->condvar.wait(
        lock,
        [this, context]() {
          return context->we_done_here
              || context->ppl.size() >= max_ppl_
              || context->pending_cells.size() < per_find_concurrency_;
        });
    if (context->ppl.size() >= max_ppl_) {
      context->we_done_here = true;
      context->condvar.wait(
          lock,
          [context]() {
            return context->pending_cells.empty();
          });
    } else if (!context->we_done_here) {
      CHECK(context->pending_cells.insert(cell).second);
      workers_->QueueWorklette([this, context, cell]() {
          TryFindPplAsync(cell, context);
        });
    }
        
    return context->we_done_here;
  }


  void TryFindPplAsync(CellLocator cell, FindContext* context) const {
    bool done;
    /* lock block */ {
      std::unique_lock<std::mutex> lock(context->mutex);
      done = context->we_done_here;
      if (done) {
        CHECK(context->pending_cells.erase(cell) == 1);
        context->condvar.notify_all();
      }
    }
          
    if (!done) {
      std::vector<Person> my_ppl;
      FindMatchingPpl(*context->parameters, ppl_[GetPplIndex(cell)], &my_ppl);
      std::unique_lock<std::mutex> lock(context->mutex);
      for (auto const& person : my_ppl)
        context->ppl.push_back(person);
      CHECK(context->pending_cells.erase(cell) == 1);
      context->condvar.notify_all();            
    }
  }

  
  void FindMatchingPpl(
      core::PplMatchingParameters const& parameters,
      std::vector<std::unique_ptr<Person>> const& ppl_cell,
      std::vector<Person>* ppl) const {
    auto const today = date_provider_();
    auto const earliest = today - boost::gregorian::years(
        parameters.age_of_user() + max_age_difference_);
    auto const latest = today - boost::gregorian::years(
        parameters.age_of_user() - max_age_difference_);

    auto person = std::lower_bound(
        begin(ppl_cell), end(ppl_cell), earliest,
        [](std::unique_ptr<Person> const& lhs, boost::gregorian::date rhs) {
          return lhs->date_of_birth() < rhs;
        });
    for (;
         person != end(ppl_cell) && (*person)->date_of_birth() <= latest;
         ++person)
      ppl->push_back(**person);
  }
};


PplmeMatchingPplProvider::PplmeMatchingPplProvider(
    int resolution,
    int max_age_difference,
    int max_ppl,
    boost::optional<int> per_find_concurrency,
    std::function<boost::gregorian::date()> date_provider) :
    impl_{new Impl{
        resolution,
        max_age_difference,
        max_ppl,
        per_find_concurrency,
        date_provider}} {}


PplmeMatchingPplProvider::~PplmeMatchingPplProvider() noexcept(true) = default;


void PplmeMatchingPplProvider::AddPerson(
    std::unique_ptr<core::Person> person) {
  impl_->AddPerson(std::move(person));
}

  
std::vector<core::Person>
PplmeMatchingPplProvider::FindMatchingPpl(
    core::PplMatchingParameters const& parameters) const {
  return impl_->FindMatchingPpl(parameters);
}


}  // namespace engine
}  // namespace pplme
