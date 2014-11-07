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


int main(int argc, char* argv[]) {
  std::string usage{"pplmed, the pplMe daemon.  Sample usage:\n"};
  usage += argv[0];
  usage += " <server listen port>";
  usage += " <size of pplMe test database>";
  usage += " <maximum age difference>";
  google::SetUsageMessage(usage);
  google::ParseCommandLineFlags(&argc, &argv, true);

  google::InitGoogleLogging(argv[0]);

  pplme::Server server(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
  if (!server.Go())
  {
    std::cerr << "Failed to start pplMe server (check logs for details)"
              << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "pplMe: Server up and running and listening on port "
            << argv[1]
            << std::endl;
  std::cout << "[stdin to continue]" << std::endl;
  std::cin.get();
  
  return EXIT_SUCCESS;
}
