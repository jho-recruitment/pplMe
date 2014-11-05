/**
 *  @file
 *  @brief   Definition of pplme::net::detail::Connection, which is an
 *           implementation detail of connection abstraction in libpplmenet.
 *  @author  j.ho
 */
#ifndef PPLME_LIBPPLMENETDETAIL_CONNECTION_H_
#define PPLME_LIBPPLMENETDETAIL_CONNECTION_H_


#include <boost/asio/ip/tcp.hpp>


namespace pplme {
namespace net {
class Message;
}  // namespace net
}  // namespace pplme


namespace pplme {
namespace net {
namespace detail {


/**
 *  @note
 *  Instances of this class typically require synchronization in terms of
 *  the coordination of sending and receiving messages (e.g., multiple threads
 *  calling SendMessage()/ReceiveMessage() are likely to experience message
 *  corruption).
 */
class Connection {
 public:
  /** Takes ownership of @a socket. */
  explicit Connection(boost::asio::ip::tcp::socket&& socket);

  /** Get the endpoint on the remote side of this connection. */
  boost::asio::ip::tcp::endpoint GetPeerEndpoint() const;

  /** Send @a message.  Returns true IFF send succeeded at the TCP/IP level. */
  bool SendMessage(Message const& message);

  /** Blocks until a message is received.  Returns non-owning on error. */
  std::unique_ptr<Message> ReceiveMessage();

 private:
  /** The connection's underlying socket. */
  boost::asio::ip::tcp::socket socket_;
};


}  // namespace detail
}  // namespace net
}  // namespace pplme


#endif  // PPLME_LIBPPLMENETDETAIL_CONNECTION_H_
