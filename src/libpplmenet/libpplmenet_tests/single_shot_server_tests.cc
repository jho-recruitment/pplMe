/**
 *  @file
 *  @brief   Tests for pplme::net::SingleShotServer.
 *  @author  j.ho
 */


#include <gtest/gtest.h>
#include "libpplmenet/single_shot_server.h"


using pplme::net::SingleShotServer;


TEST(SingleShotServerTest, TestPortInUse) {
  SingleShotServer server1{0, SingleShotServer::RequestHandler{}};
  ASSERT_TRUE(server1.Start());

  SingleShotServer server2
      {server1.GetLocalPort(), SingleShotServer::RequestHandler{}};
  ASSERT_FALSE(server2.Start());
}
