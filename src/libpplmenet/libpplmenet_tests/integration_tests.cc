/**
 *  @file
 *  @brief   Integration Tests for libpplmenet.
 *  @author  j.ho
 */


#include <gtest/gtest.h>
#include "libpplmenet/client.h"
#include "libpplmenet/message.h"
#include "libpplmenet/single_shot_server.h"


using pplme::net::Client;
using pplme::net::SingleShotServer;


namespace {


std::unique_ptr<pplme::net::Message> CreatePxng(char x) {
  auto body = pplme::net::Message::CreateBodyBuffer(4);
  memcpy(body.get(), "PANG", 4);
  static_cast<unsigned char*>(body.get())[1] = x;
  std::unique_ptr<pplme::net::Message> message{
    new pplme::net::Message{std::move(body), 4}};
  return message;
}


std::unique_ptr<pplme::net::Message> PingPongRequestHandler(
    std::string const&,
    unsigned short,
    pplme::net::Message const& request) {
  if (memcmp(request.GetBodyOctets(), "PING", 4) != 0)
    // Can't seem to get ASSERT_stuffs to work here, so we get drastic:
    *(char*)nullptr = 0;
  return CreatePxng('O');
}


}  // namespace


TEST(libpplmenetTest, RequestResponseCycle)
{
  SingleShotServer server{0, PingPongRequestHandler};
  server.Start();

  Client client{"127.0.0.1", server.GetLocalPort()};
  client.Connect();
  auto response = client.SendRequest(*CreatePxng('I'));

  ASSERT_EQ(0, memcmp(response->GetBodyOctets(), "PONG", 4));
}
