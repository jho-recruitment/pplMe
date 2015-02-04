/**
 *  @file
 *  @brief   Implementation for pplme::net::SingleShotServer.
 *  @author  j.ho
 */


#include "single_shot_server.h"
#include <mutex>
#include <thread>
#include <boost/asio.hpp>
#include <glog/logging.h>
#include "message.h"
#include "detail/connection.h"


using batcpip = boost::asio::ip::tcp;
using boost::system::error_code;


namespace pplme {
namespace net {


/**
 *  This implementation is kinda simple in that it uses a
 *  one-thread-per-connection model.  This obviously doesn't scale well, but
 *  is surely good enough for the humble purposes of pplMe.
 */
class SingleShotServer::Impl {
 public:
  explicit Impl(unsigned short port, RequestHandler request_handler) :
      request_handler_{request_handler},
      io_service_token_work_{io_service_},
      endpoint_{batcpip::v4(), port},
      acceptor_{io_service_} {}


  ~Impl() {
    Shutdown();
  }
  
  
  bool Start() {
    error_code error;
    acceptor_.open(endpoint_.protocol(), error);
    if (!error)
      acceptor_.bind(endpoint_, error);
    if (!error)
      acceptor_.listen(boost::asio::socket_base::max_connections, error);
    if (error) {
      LOG(ERROR) << "Failed to initialize a local listening socket: "
                 << error << ": " << error.message();
    }

    bool success = !error;
    if (success) {
      LOG(INFO) << "SingleShotServer is listening for connections on "
                << endpoint_;
    }
 
    try {
      if (success)
        io_thread_ = std::thread([this]() { GoIoServiceGo(); });
    } catch (std::system_error const& error) {
      success = false;
    }

    if (success) {
      // Prepare to start tracking connections.
      connection_threads_ =
          std::make_unique<std::map<std::thread::id, std::thread>>();

      // Kick off the first accept (which is asynchronously "recursive").
      KickOffAccept();
    }

    return success;
  }


  unsigned short GetLocalPort() const {
    return acceptor_.local_endpoint().port();
  }
  
  
  void Shutdown() {
    // Take ownership of the connection threads so that we can wait for them
    // all to finish and prevent any more starting.
    decltype(connection_threads_) threads;
    /* lock block */ {
      std::unique_lock<std::mutex> lock(connection_threads_lock_);
      threads = std::move(connection_threads_);
    }

    if (threads) {
      LOG(INFO) << "Initiating shutdown of SingleShotServer ...";

      for (auto& thread : *threads)
        thread.second.join();

      io_service_.stop();
      io_thread_.join();

      LOG(INFO) << "Shutdown of SingleShotServer is complete.";
    }
  }

  
 private:
  /** The object that handles a client's request. */
  RequestHandler request_handler_;
  /** The ASIO io_service used to process this connection. */
  boost::asio::io_service io_service_;
  /** Token work to keep io_service_ busy. */
  boost::asio::io_service::work io_service_token_work_;
  /** The thread that spins work for io_service_. */
  std::thread io_thread_;
  /** The server's local listening endpoint. */
  batcpip::endpoint endpoint_;
  /** The ASIO magic responsible for processing client connections. */
  batcpip::acceptor acceptor_;
  /** Lock for messing with connection_threads_. */
  std::mutex connection_threads_lock_;
  /** This is used for keeping track of all the active connection threads.
      If it is non-owning, then either we haven't been Start()ed, or we've
      already been Shutdown(). */
  std::unique_ptr<std::map<std::thread::id, std::thread>> connection_threads_;

  
  void GoIoServiceGo() {
    boost::system::error_code error;
    io_service_.run(error);
    if (error) {
      LOG(ERROR) << "SingleShotServer I/O loop stopped unexpectedly: "
                 << error << ": " << error.message();
    }
  }


  void KickOffAccept() {
    // Ideally this would be std::make_unique'd, but due to the fact that
    // the handler function object may be copied, that doesn't work.
    auto socket = std::make_shared<batcpip::socket>(io_service_);
    acceptor_.async_accept(
        *socket,
        [this, socket](boost::system::error_code const& error) {
          HandleAcceptResult(socket, error);
          KickOffAccept();
        });
  }     


  void HandleAcceptResult(std::shared_ptr<batcpip::socket> socket,
                          error_code const& error) {
    if (!error) {
      auto connection =
          std::make_shared<detail::Connection>(std::move(*socket));

      std::unique_lock<std::mutex> lock(connection_threads_lock_);
      if (connection_threads_) {
        std::thread connection_thread{
            [this, connection]() { Service(connection); }};
        connection_threads_->emplace(connection_thread.get_id(),
                                      std::move(connection_thread));
      } else {
        LOG(WARNING) << "Not processing connection from "
                     << socket->remote_endpoint()
                     << " due to SingleShotServer shutting down";
      }
    } else
      LOG(ERROR) << "Accept error: " << error << ": " << error.message();
  }

  
  void Service(std::shared_ptr<detail::Connection> connection) 
  {
    LOG(INFO) << "Connection from " << connection->GetPeerEndpoint();

    // First, we try to receive a message....
    auto request = connection->ReceiveMessage();
    if (request) {
      auto response = request_handler_(
          connection->GetPeerEndpoint().address().to_string(),
          connection->GetPeerEndpoint().port(),
          *request);
      if (response) {
        // ...then we send our response.
        if (!connection->SendMessage(*response)) {
          LOG(INFO) << "Failed to send response to "
                    << connection->GetPeerEndpoint();
        }
      }
    }     else {
      LOG(INFO) << "Failed to receive request from "
                << connection->GetPeerEndpoint();
    }

    // Heheh.  But more seriously, we need to do this so that this thread
    // no-longer owns anything because once we remove it from the collection
    // of connection threads, we need to be certain that it can't touch
    // anything that may very well be about to be destroyed.
    connection.reset();

    std::thread me;
    /* lock block */ {
      std::unique_lock<std::mutex> lock(connection_threads_lock_);
      auto my_entry = connection_threads_->find(std::this_thread::get_id());
      me = std::move(my_entry->second);
      connection_threads_->erase(my_entry);
    }
    me.detach();
  }
};


SingleShotServer::SingleShotServer(unsigned short port,
                                   RequestHandler request_handler) :
    impl_{new Impl{port, request_handler}} {}


SingleShotServer::~SingleShotServer() = default;


bool SingleShotServer::Start() {
  return impl_->Start();
}


unsigned short SingleShotServer::GetLocalPort() const {
  return impl_->GetLocalPort();
}


void SingleShotServer::Shutdown() {
  impl_->Shutdown();
}


}  // namespace net
}  // namespace pplme
