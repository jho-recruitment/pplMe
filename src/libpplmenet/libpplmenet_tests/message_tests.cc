/**
 *  @file
 *  @brief   Tests for pplme::net::Message.
 *  @author  j.ho
 */


#include <gtest/gtest.h>
#include "libpplmenet/message.h"



using pplme::net::Message;


/**
 *  @test
 *  @remarks  With the current implementation, it's important that this is
 *            maintained as an invariant else we'll break backward
 *            compatibility.  So we double-check here.
 */
TEST(MessageTest, HeaderSizeIsFixed) {
  ASSERT_EQ(4U, sizeof(Message::Header));
}


/**
 *  @test
 *  @remarks  With the current implementation, it's important that this is
 *            maintained as an invariant else we'll break backward
 *            compatibility.
 */
TEST(MessageTest, HeaderBodyLengthIsInNetworkOrder) {
  Message::Header message_header{0x1234U};

  ASSERT_EQ(0x1234U, ntohl(*reinterpret_cast<uint32_t *>(&message_header)));
}
