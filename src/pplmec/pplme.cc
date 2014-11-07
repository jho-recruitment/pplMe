/**
 *  @file
 *  @brief   Definition of pplme::Pplme().
 *  @author  j.ho
 */


#include "pplme.h"
#include "libpplmenet/client.h"
#include "libpplmenet/message.h"

#include "libpplmeproto/convert_person.h"
#include "libpplmeproto/pplme_response.pb.h"
#include "libpplmeproto/request.pb.h"

#include "libpplmecore/person.h"

#include <boost/date_time/gregorian/gregorian.hpp>

#include <boost/numeric/conversion/cast.hpp>




namespace pplme {


bool Pplme(
    std::string const& pplme_server_address,
    unsigned short pplme_server_port,
    float users_latitude,
    float users_longitude,
    int users_age) {

  // Create client and connect to server.
  net::Client client{pplme_server_address, pplme_server_port};
  if (!client.Connect()) {
    std::cerr << "Failed to connect to pplMe server "
              << pplme_server_address << ":" << pplme_server_port;
    return false;
  }

  // Populate PplmeRequest protobuf message.
  proto::Request request_pb;
  auto location_of_user =
      request_pb.mutable_pplme_request()->mutable_location_of_user();
  location_of_user->set_latitude(users_latitude);
  location_of_user->set_longitude(users_longitude);
  request_pb.mutable_pplme_request()->set_age_of_user(users_age);

  // Serialize PplmeRequest protobuf message into a generic pplMe message.
  auto request_body = net::Message::CreateBodyBuffer(request_pb.ByteSize());
  request_pb.SerializeToArray(request_body.get(), request_pb.ByteSize());
  net::Message request{
    std::move(request_body),
    boost::numeric_cast<uint32_t>(request_pb.ByteSize())};

  // Send request and grab response.
  auto response = client.SendRequest(request);
  if (!response) {
    std::cerr << "pplMe request to "
              << pplme_server_address << ":" << pplme_server_port
              << " failed (check logs for details)";
    return false;
  }

  // Decode generic pplMe response message into PplmeResponse protobuf message.
  proto::PplmeResponse response_pb;
  if (!response_pb.ParseFromArray(response->GetBodyOctets(),
                                  response->GetHeader().GetBodyLength())) {
    std::cerr << "Invalid pplMe response received from "
              << pplme_server_address << ":" << pplme_server_port;
    return false;
  }

  std::cout << "pplMe for user, " << users_age << " @ "
            << users_latitude << ", " << users_longitude
            << std::endl;
  if (response_pb.ppl_size() > 0) {
    std::cout << "pplMe: you have "
              << response_pb.ppl_size() << " potential friends :)"
              << std::endl;
  } else
    std::cout << "pplMe: no matching ppl found :(" << std::endl;

  for (int n = 0; n < response_pb.ppl_size(); ++n) {
    core::Person person;
    if (proto::Convert(response_pb.ppl(n), &person)) {
      auto age =
          boost::gregorian::day_clock::local_day() - person.date_of_birth();
      // From memory....  &:S
      int age_in_years = age.days() / 365.24;
      std::cout << person.name() << ", " << age_in_years << " @ "
                << person.location_of_home().latitude().value() << ", "
                << person.location_of_home().longitude().value()
                << std::endl;
    } else
      std::cout << "pplMe: warning: failed to grok person result" << std::endl;
  }

  return true;
}


}  // namespace pplme
