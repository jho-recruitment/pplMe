/**
 *  @file
 *  @brief   Tests for pplme::net::Client.
 *  @author  j.ho
 */


#include <gtest/gtest.h>
#include "libpplmenet/client.h"


using pplme::net::Client;


TEST(ClientTest, ConnectToInvalidAddress) {
  Client client{"test.invalid.", 80};

  ASSERT_FALSE(client.Connect());
}


TEST(ClientTest, ConnectToInvalidPort) {
  // O&;D
  Client client{"localhost", 11997};

  ASSERT_FALSE(client.Connect());
}
