/**
 *  @file
 *  @brief   Implementation for pplme::net::Client.
 *  @author  j.ho
 */


#include "client.h"
#include <algorithm>
#include <thread>
#include <system_error>
#include <boost/asio.hpp>
#include <boost/scoped_ptr.hpp>
#include <glog/logging.h>
#include "message.h"
#include "detail/connection.h"


using batcpip = boost::asio::ip::tcp;
using boost::system::error_code;


namespace pplme {
namespace net {


class Client::Impl {
 public:
  Impl(std::string const& address, unsigned short port) :
      address_{address},
      port_{port},
      io_service_token_work_{io_service_} {}
    

  ~Impl() {
    io_service_.stop();
    io_thread_.join();
  }

  
  bool Connect() {
    bool success = true;

    // Get the ASIO io_service up and running.
    try {
      io_thread_ = std::thread([this]() { GoIoServiceGo(); });
    } catch (std::system_error const& error) {
      success = false;
    }

    if (success) {
      // Try to resolve the server address and if successful, connect to it.
      auto endpoint = Resolve();
      if (endpoint != batcpip::endpoint()) {
        batcpip::socket socket{io_service_};
        error_code error;
        socket.connect(endpoint, error);
        if (!error)
          connection_.reset(new detail::Connection{std::move(socket)});
        else {
          LOG(ERROR) << "Failed to connect to " << address_ << ":" << port_
                     << ": " << error << ": " << error.message();
        }
        success = !error;
      }
    }

    return success;
  }


  std::unique_ptr<Message> SendRequest(Message const& request) {
    std::unique_ptr<Message> response;

    if (connection_->SendMessage(request))
      response = connection_->ReceiveMessage();

    return response;
  }


  void Disconnect() {
    connection_.reset();
  }

  
 private:
  /** The address to connect to. */
  std::string address_;
  /** The port to connect to. */
  unsigned short port_;
  /** The ASIO io_service used to process this connection. */
  boost::asio::io_service io_service_;
  /** Token work to keep io_service_ busy. */
  boost::asio::io_service::work io_service_token_work_;
  /** The thread that spins work for io_service_. */
  std::thread io_thread_;
  /** The actual connection (null if we have not successfully connected or
      we have disconnected). */
  boost::scoped_ptr<detail::Connection> connection_;


  void GoIoServiceGo() {
    boost::system::error_code error;
    io_service_.run(error);
    if (error) {
      LOG(ERROR) << "Server I/O loop stopped unexpectedly: "
                 << error << ": " << error.message();
    }
  }
      
      
  batcpip::endpoint Resolve() {
    batcpip::resolver resolver{io_service_};
    // Dear oh dear, this is a tad tedious.  &:/
    std::ostringstream service;
    service.imbue(std::locale::classic());
    service << port_;
    batcpip::resolver::query query(address_, service.str());

    VLOG(1) << "Resolving " << address_ << ":" << port_ << " ...";
    
    error_code error;
    std::vector<batcpip::endpoint> endpoints{
        resolver.resolve(query, error),
        batcpip::resolver::iterator()};

    if (!error) {
      if (VLOG_IS_ON(1)) {
        for (auto const& endpoint : endpoints) {
          VLOG(1) << "Resolved " << address_ << ":" << port_
                  << " to " << endpoint;
        }
        VLOG(1) << "Done resolving " << address_ << ":" << port_;
      }

      // Currently, we only do IPv4.
      auto is_v4 = [](batcpip::endpoint const& endpoint) {
        return endpoint.protocol() != batcpip::v4();
      };
      endpoints.erase(
          std::remove_if(endpoints.begin(), endpoints.end(), is_v4),
          endpoints.end());
                              
      if (endpoints.empty()) {
        LOG(ERROR) << "Failed to resolve " << address_ << ":" << port_
                   << " to any IPv4 addresses";
      }
      else if (endpoints.size() > 1) {
        LOG(WARNING) << "Resolving " << address_ << ":" << port_
                     << " gave multiple results; only using first";
      }
    }
    
    return endpoints.empty() ? batcpip::endpoint{} : endpoints[0];
  }
};


Client::Client(std::string const& address, unsigned short port) :
    impl_{new Impl{address, port}} {}


Client::~Client() = default;


bool Client::Connect() {
  return impl_->Connect();
}


std::unique_ptr<Message> Client::SendRequest(Message const& request) {
  return impl_->SendRequest(request);
}


void Client::Disconnect() {
  impl_->Disconnect();
}


}  // namespace net
}  // namespace pplme
