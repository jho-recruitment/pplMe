/**
 *  @file
 *  @brief   Implementation for pplme::net::detail::Connection.
 *  @author  j.ho
 */


#include "connection.h"
#include <boost/asio.hpp>
#include <glog/logging.h>
#include "../message.h"


using batcpip = boost::asio::ip::tcp;
using boost::system::error_code;


namespace pplme {
namespace net {
namespace detail {


Connection::Connection(batcpip::socket&& socket) :
    socket_{std::move(socket)} {}


batcpip::endpoint Connection::GetPeerEndpoint() const {
  return socket_.remote_endpoint();
}


bool Connection::SendMessage(Message const& message) {
  // First write the Message's header...
  auto const* header = &message.GetHeader();
  auto header_octets = boost::asio::buffer(header, sizeof(*header));
  error_code error;
  boost::asio::write(socket_, header_octets, error);
  if (!error) {
    // ...then write the Message's body.
    auto body_octets = boost::asio::buffer(message.GetBodyOctets(),
                                           message.GetBodyLength());
    boost::asio::write(socket_, body_octets, error);
  }

  if (error) {
    LOG(ERROR) << "Failed to send message to " << socket_.remote_endpoint()
               << ": " << error;
  }     
  
  return !error;
}


std::unique_ptr<Message> Connection::ReceiveMessage() {
  std::unique_ptr<Message> message;

  // First read the Message's header...
  Message::Header header;
  auto header_octets = boost::asio::buffer(&header, sizeof(header));
  error_code error;
  boost::asio::read(socket_, header_octets, error);
  if (!error) {
    // ...then receive the Message's body.
    if (header.GetBodyLength() <= Message::kMaxBodyLength) {
      auto body_octets = Message::CreateBodyBuffer(header.GetBodyLength());
      boost::asio::read(
          socket_,
          boost::asio::buffer(body_octets.get(), header.GetBodyLength()),
          error);
      if (!error)
        message.reset(new Message{header, std::move(body_octets)});
    }
    else {
      LOG(ERROR) << "Message received from " << socket_.remote_endpoint()
                 << " was too big at " << header.GetBodyLength() << " octets";
    }     
  }

  if (error) {
    LOG(ERROR) << "Failed to receive message from " << socket_.remote_endpoint()
               << ": " << error;
  }
  
  return message;
}


}  // namespace detail
}  // namespace net
}  // namespace pplme
