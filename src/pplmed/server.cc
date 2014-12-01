/**
 *  @file
 *  @brief   Implementation for pplme::Server.
 *  @author  j.ho
 */


#include "server.h"
#include <chrono>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/uuid/random_generator.hpp>
#include <glog/logging.h>
#include "libpplmeengine/ppl_slurper.h"
#include "libpplmeengine/pplme_matching_ppl_provider.h"
#include "libpplmenet/message.h"
#include "libpplmenet/single_shot_server.h"
#include "libpplmeproto/convert_geo_position.h"
#include "libpplmeproto/convert_person.h"
#include "libpplmeproto/pplme_response.pb.h"
#include "libpplmeproto/request.pb.h"


namespace {


// Timezones have a tendency to be tricky.  We wuss out here and just opt for
// localtime.  &:/
boost::gregorian::date GetTodaysDate() {
  return boost::gregorian::day_clock::local_day();
}


}  // namespace


namespace pplme {


class Server::Impl {
 public:
  Impl(
      int port,
      int test_db_size,
      int grid_resolution,
      int max_ppl,
      int max_age_difference,
      std::string const& ppldata_filename) :
      test_db_size_{test_db_size},
      ppldata_filename_{ppldata_filename},
      matching_ppl_provider_{
          grid_resolution,
          max_age_difference,
          max_ppl,
          boost::none,    
          &GetTodaysDate},
      pplme_requests_server_{
          boost::numeric_cast<unsigned short>(port),
          std::bind(&Impl::HandlePplmeRequest,
                    this,
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3)} {}

  
  ~Impl() {
    pplme_requests_server_.Shutdown();
  }

  
  bool Go() {
    bool ok = true;

    if (!ppldata_filename_.empty()) {
      engine::PplSlurper slurper{ppldata_filename_};
      LOG(INFO) << "Loading ppl data from `" << ppldata_filename_ << "'...";
      if (!slurper.Populate(&matching_ppl_provider_)) {
        LOG(ERROR) << "Failed to load ppl data from `"
                   << ppldata_filename_ << "'";
        ok = false;
      }
    }
    else {
      LOG(INFO) << "Generating ppl test data...";
      PopulateTestDb();
    }
    
    ok = ok && pplme_requests_server_.Start();

    return ok;
  }

  
 private:
  int test_db_size_;
  std::string ppldata_filename_;
  engine::PplmeMatchingPplProvider matching_ppl_provider_;
  net::SingleShotServer pplme_requests_server_;

  
  void PopulateTestDb() {
    boost::uuids::random_generator random_uuid_generator;

    std::default_random_engine random_engine;
    std::uniform_int_distribution<int> random_age{-100 * 365, -18 * 365};
    std::uniform_real_distribution<float> random_latitude{-90, 90};
    std::uniform_real_distribution<float> random_longitude{-180, 180};

    for (int i = 0; i < test_db_size_; ++i) {
      auto random_dob = GetTodaysDate() +
          boost::gregorian::days{random_age(random_engine)};
      core::GeoPosition random_location{
          core::GeoPosition::DecimalLatitude{random_latitude(random_engine)},
          core::GeoPosition::DecimalLongitude{random_longitude(random_engine)}};
      std::unique_ptr<core::Person> person{new core::Person{
          core::PersonId{random_uuid_generator()},
          std::string{u8"John Malkovich " + std::to_string(i)},
              random_dob,
                  random_location}};

      matching_ppl_provider_.AddPerson(std::move(person));
    }
  }


  std::unique_ptr<net::Message> HandlePplmeRequest(
    std::string const& address,
    unsigned short port,
    net::Message const& request)
  {
    auto const addressnport = [address, port]() {
      std::ostringstream oss;
      oss << address << ":" << port;
      return oss.str();
    }();

    // First off, do some basic validation on the protobuf request.
    proto::Request request_pb;
    if (!request_pb.ParseFromArray(request.GetBodyOctets(),
                                   request.GetHeader().GetBodyLength())) {
      LOG(WARNING) << "Ignoring malformed request from " << addressnport;
      return std::unique_ptr<net::Message>{};
    }
    if (!request_pb.has_pplme_request()) {
      LOG(WARNING) << "Ignoring unknown request type from " << addressnport;
      return std::unique_ptr<net::Message>{};
    }
    core::GeoPosition location_of_user;
    if (!proto::Convert(request_pb.pplme_request().location_of_user(),
                        &location_of_user)) {
      LOG(WARNING) << "Ignoring invalid location-of-user in PplmeRequest from "
                   << addressnport;
      return std::unique_ptr<net::Message>{};
    }

    auto then = std::chrono::high_resolution_clock::now();
    
    // Okay, the PplmeRequest looks sound, let's see who we can find...
    LOG(INFO) << "Processing request from " << addressnport
              << " from user, " << request_pb.pplme_request().age_of_user()
              << " @ " << location_of_user.latitude()
              << ", " << location_of_user.longitude();
    auto matching_ppl = matching_ppl_provider_.FindMatchingPpl(
        core::PplMatchingParameters{
            location_of_user, request_pb.pplme_request().age_of_user()});

    auto now = std::chrono::high_resolution_clock::now();
    auto took = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - then);
    VLOG(1) << "FindMatchinPpl() took " << took.count();
    
    // ...and smash each one into a PplmeResponse.
    proto::PplmeResponse response_pb;
    for (auto const& person : matching_ppl) {
      auto person_pb = response_pb.mutable_ppl()->Add();
      proto::Convert(person, person_pb);
    }
    auto response_body = net::Message::CreateBodyBuffer(response_pb.ByteSize());
    response_pb.SerializeToArray(response_body.get(), response_pb.ByteSize());

    // Finally, return the PplmeResponse framed as a generic pplMe Message.  &%D
    return std::unique_ptr<net::Message>{new net::Message{
        std::move(response_body),
        boost::numeric_cast<uint32_t>(response_pb.ByteSize())}};
  }
};


Server::Server(
    int port,
    int test_db_size,
    int grid_resolution,
    int max_distance,
    int max_age_difference,
    std::string const& ppldata_filename) :
    impl_{new Impl{
        port,
        test_db_size,
        grid_resolution,
        max_distance,
        max_age_difference,
        ppldata_filename}} {}


bool Server::Go() {
  return impl_->Go();
}


Server::~Server() = default;


}  // namespace pplme
