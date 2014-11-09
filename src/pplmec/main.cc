/**
 *  @file
 *  @brief   Entry point for pplmec, the pplMe command-line client.
 *  @author  j.ho
 */


#include <stdlib.h>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include "pplme.h"


using google::RegisterFlagValidator;


DEFINE_string(server,
              "localhost",
              "address of the pplMe server");
extern bool const server_validation_registrar = RegisterFlagValidator(
    &FLAGS_server,
    [](char const*, std::string const& value) {
      return !value.empty();
    });

DEFINE_int32(port,
             3333,
             "port of the pplMe server");
extern bool const port_validation_registrar = RegisterFlagValidator(
    &FLAGS_port,
    [](char const*, int32_t value) {
      return value > 0 && value <= 65535;
    });

DEFINE_double(latitude,
              91.0,
              "user's decimal latitude");
extern bool const latitude_validation_registrar = RegisterFlagValidator(
    &FLAGS_latitude,
    [](char  const*, double value) {
      return value >= -90 && value <= 90;
    });

DEFINE_double(longitude,
              -181.0,
              "user's decimal longitude");
extern bool const longitude_validation_registrar = RegisterFlagValidator(
    &FLAGS_longitude,
    [](char const*, double value) {
      return value >= -180 && value <= 180;
    });

DEFINE_int32(age,
             -1,
             "user's age");
extern bool const age_validation_registrar = RegisterFlagValidator(
    &FLAGS_age,
    [](char const*, int32_t age) {
      return age >= 0;
    });


int main(int argc, char* argv[]) {
  std::string usage{"pplmec, the pplMe client.  Sample usage:\n"};
  usage += argv[0];
  usage += " --latitude 12.34 --longitude --56.789 --age 100";
  google::SetUsageMessage(usage);
  google::ParseCommandLineFlags(&argc, &argv, true);

  google::InitGoogleLogging(argv[0]);

  auto pplmed = pplme::Pplme(
      FLAGS_server,
      FLAGS_port,
      FLAGS_latitude,
      FLAGS_longitude,
      FLAGS_age);
  
  return pplmed ? EXIT_SUCCESS : EXIT_FAILURE;
}
