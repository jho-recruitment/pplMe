/**
 *  @file
 *  @brief   Definition of pplme::net::Client, which provides a simple
 *           abstraction of a client-initiated TCP/IP connection to a server.
 *  @author  j.ho
 */
#ifndef PPLME_LIBPPLMENET_CLIENT_H_
#define PPLME_LIBPPLMENET_CLIENT_H_


#include <memory>
#include <string>
#include "libpplmeutils/pimpl.h"


namespace pplme {
namespace net {
class Message;
}  // namespace net
}  // namespace pplme


namespace pplme {
namespace net {


/**
 *  Example:
 *  @code  
 *  Client client("pplme.example.com", 3333);
 *
 *  if (client.Connect()) {
 *    while (!message_q.empty()) {
 *      auto response = client.SendRequest(message_q.front());
 *      if (response) {
 *        message_q.pop_front();
 *        //[...]
 *      } else {
 *        // Oh dear, sommat's gone wrong.
 *        break;
 *      }
 *    }
 *    
 *    client.Disconnect();
 *  }
 *  @endcode
 *
 *  @remarks
 *  This class currently only supports single-request -> single-response
 *  messaging scenarios.
 *
 *  @remarks
 *  This class currently only supports connections to IPv4-routable hosts.
 *
 *  @note
 *  This class is not safe to use in an unsychronized manner; that is,
 *  calls to Connect()/SendRequest()/Disconnect() should be appropriately
 *  ordered and should not occur concurrently.
 */
class Client {
 public:
  /**
   *  @param address is the hostname, DNS name, or IP address of the TCP/IP
   *         server host to connect to.
   *  @param port is the TCP/IP port number to connect to.
   */
  Client(std::string const& address, unsigned short port);
  ~Client();

  /**
   *  Actually connect to the server.
   *
   *  @returns true IFF a connection was successfully established.
   *
   *  @remarks
   *  Should be called at most once, before any calls to SendRequest() and/or
   *  Disconnect().
   */
  bool Connect();

  /**
   *  Send @a request message to the server and wait for a response.
   *
   *  @param request is the message to send.
   *  @returns the resulting response message from the server; a non-owning
   *           std::unique_ptr indicates an error condition (i.e., the
   *           connection has been lost).
   *
   *  @remarks
   *  May be called multiple times so long as Connect() has already been called
   *  and returned success and Disconnect() has not.
   */
  std::unique_ptr<Message> SendRequest(Message const& request);

  /**
   *  Disconnect from the server.
   *
   *  @remarks
   *  Should only be called at most once and only after a successful call to
   *  Connect().
   */
  void Disconnect();
  
 private:
  class Impl;
  utils::Pimpl<Impl> impl_;
};


}  // namespace net
}  // namespace pplme


#endif  // PPLME_LIBPPLMENET_CLIENT_
