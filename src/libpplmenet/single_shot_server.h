/**
 *  @file
 *  @brief   Definition of pplme::net::SingleShotServer, which provides a simple
 *           abstraction of a TCP/IP server that handles a single request (and
 *           returns a single response) per client connection.
 *  @author  j.ho
 */
#ifndef PPLME_LIBPPLMENET_SINGLESHOTSERVER_H_
#define PPLME_LIBPPLMENET_SINGLESHOTSERVER_H_


#include <functional>
#include <memory>
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
 *  auto handle_null_server_request = [](
 *      std::string const& address,
 *      unsigned short port,
 *      Message const& request) {
 *    LOG(INFO) << "Processing request of length " << request.GetBodyLength()
 *              << " from " << address << ":" << port;  
 *    std::unique_ptr<uint8_t[]> response_body{new uint8_t[0]};
 *    return std::unique_ptr<Message>{new Message{std::move(response_body), 0}};
 *  };
 *
 *  SingleShotServer null_server{port, handle_null_server_request};
 *
 *  if (null_server.Start()) {
 *    // Token stop signal:
 *    std::cin.get();
 *    null_server.Shutdown();
 *  }
 *  @endcode
 *
 *  @remarks
 *  This class currently only supports connections from IPv4-routable hosts.
 *
 *  @note
 *  This class is not safe to use in an unsychronized manner; that is,
 *  calls to Connect()/SendRequest()/Disconnect() should be appropriately
 *  ordered and should not occur concurrently.
 */
class SingleShotServer {
 public:
  /**
   *  Function object type that is used for handling requests sent by clients.
   *
   *  This handler is inboked once per each client connection (assuming that
   *  the client is not disconnected before it manages to send a request)
   *  and is given the address:port of the client along with the client's
   *  request.  The handler is then responsible for returning a Message that
   *  should be sent back to the client as a respose to the request.
   */
  typedef
    std::function<std::unique_ptr<Message> (std::string const& address,
                                            unsigned short port,
                                            Message const& request)>
    RequestHandler;

  /** Create a server that will listen on @a port, where if that port is 0,
      dynamically assign a port. */
  SingleShotServer(unsigned short port, RequestHandler request_handler);
  ~SingleShotServer();

  /** Start the server listening for client connections and hence requests
      to process. */
  bool Start();

  /** Discover what port we bound to.  Useful when 0 was passed to the ctor.
      Can only be called inbetween Start() and Shutdown(). */
  unsigned short GetLocalPort() const;
  
  /** Shut the server down. */
  void Shutdown();
  
 private:
  class Impl;
  utils::Pimpl<Impl> impl_;
};


}  // namespace net
}  // namespace pplme


#endif  // PPLME_LIBPPLMENET_SINGLESHOTSERVER_H_
