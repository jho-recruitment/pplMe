/**
 *  @file
 *  @brief   Entry point for pplmed, the pplMe daemon.
 *  @author  j.ho
 */


#include <stdlib.h>
#include <iostream>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include "server.h"


using google::RegisterFlagValidator;


DEFINE_int32(port,
             3333,
             "port to listen on");
extern bool const port_validation_registrar = RegisterFlagValidator(
    &FLAGS_port,
    [](char const*, int32_t value) {
      return value > 0 && value <= 65535;
    });

DEFINE_int32(test_database_size,
             10000000,
             "size of the test database");
extern bool const test_database_size_validation_registrar =
    RegisterFlagValidator(
        &FLAGS_test_database_size,
        [](char const*, int32_t value) {
          return value >= 0;
        });

DEFINE_int32(grid_resolution,
             10,
             "number of cells per decimal degree in the grid");
extern bool const grid_resolution_validation_registrar =
    RegisterFlagValidator(
        &FLAGS_grid_resolution,
        [](char const*, int32_t value) {
          return value > 0 && value <= 100;
        });

DEFINE_int32(max_distance,
             10,
             "maximum distance of matches in km");
extern bool const max_distance_validation_registrar =
    RegisterFlagValidator(
        &FLAGS_max_distance,
        [](char const*, int32_t value) {
          return value > 0;
        });

DEFINE_int32(max_age_difference,
             10,
             "maximum age difference of matches");
extern bool const max_age_difference_validation_registrar =
    RegisterFlagValidator(
        &FLAGS_max_age_difference,
        [](char const*, int32_t value) {
          return value >= 0;
        });

DEFINE_string(ppldata,
              "",
              "path to a CSV file containing data for the pplMe database");


int main(int argc, char* argv[]) {
  std::string usage{"pplmed, the pplMe daemon.  Sample usage:\n"};
  usage += argv[0];
  usage += " --port 6666 --max_age_difference 20 --ppldata pplMe-data.csv";
  google::SetUsageMessage(usage);
  google::ParseCommandLineFlags(&argc, &argv, true);

  google::InitGoogleLogging(argv[0]);

  pplme::Server server(
      FLAGS_port,
      FLAGS_test_database_size,
      FLAGS_grid_resolution,
      FLAGS_max_distance,
      FLAGS_max_age_difference,
      FLAGS_ppldata);
  if (!server.Go())
  {
    std::cerr << "Failed to start pplMe server (check logs for details)"
              << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "pplMe: Server up and running and listening on port "
            << FLAGS_port
            << std::endl;
  std::cout << "[stdin to continue]" << std::endl;
  std::cin.get();
  
  return EXIT_SUCCESS;
}
