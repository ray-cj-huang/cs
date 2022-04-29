#ifndef server_h
#define server_h

#include <boost/asio.hpp>
#include "session.h"
#include "logger.h"
#include "echo_request_handler.h"
#include "static_request_handler.h"

using boost::asio::ip::tcp;

class server
{
  friend class ServerTest;
public:
  server(boost::asio::io_service& io_service,
         short port,
         echo_request_handler* echo_request_handler,
         static_request_handler* static_request_handler);
         
  enum class HandleAcceptFlag { START, OK, ERROR };

private:
  void start_accept();
  void handle_accept(session* new_session,
      const boost::system::error_code& error);

  boost::asio::io_service& io_service_;
  tcp::acceptor acceptor_;
  HandleAcceptFlag flag = HandleAcceptFlag::START;
  echo_request_handler* echo_request_handler_;
  static_request_handler* static_request_handler_;
};

#endif
