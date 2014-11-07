/**
 *  @file
 *  @brief   Entry point for pplmec, the pplMe command-line client.
 *  @author  j.ho
 */


#include <stdlib.h>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include "pplme.h"


int main(int argc, char* argv[]) {
  std::string usage{"pplmec, the pplMe client.  Sample usage:\n"};
  usage += argv[0];
  usage += " <pplMe server> <pplMe port>";
  usage += " <user's decimal latitude> <user's decimal longitude> <user's age>";
  google::SetUsageMessage(usage);
  google::ParseCommandLineFlags(&argc, &argv, true);

  google::InitGoogleLogging(argv[0]);

  auto pplmed = pplme::Pplme(
      argv[1],
      atoi(argv[2]),
      atof(argv[3]),
      atof(argv[4]),
      atoi(argv[5]));
  
  return pplmed ? EXIT_SUCCESS : EXIT_FAILURE;
}
