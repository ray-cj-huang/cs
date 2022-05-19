#ifndef server_h
#define server_h

#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <unordered_map>
#include <vector>

#include "session.h"
#include "logger.h"
#include "request_handler_factory.h"

using boost::asio::ip::tcp;

class server
{
  friend class ServerTest;
public:
  server(boost::asio::io_service& io_service, short port, 
         std::unordered_map<std::string, request_handler_factory*> routes, size_t thread_pool_size);
  void run();
  
  enum class HandleAcceptFlag { START, OK, ERROR };

private:
  void start_accept();
  void handle_accept(session* new_session,
      const boost::system::error_code& error);

  boost::asio::io_service& io_service_;
  tcp::acceptor acceptor_;
  HandleAcceptFlag flag = HandleAcceptFlag::START;
  std::unordered_map<std::string, request_handler_factory*> routes_;
  std::vector<boost::shared_ptr<boost::thread>> threads_;
  size_t thread_pool_size_;
};

#endif
