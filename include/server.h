#ifndef server_h
#define server_h

#include <boost/asio.hpp>
#include "session.h"

using boost::asio::ip::tcp;

class server
{
  friend class ServerTest;
public:
  server(boost::asio::io_service& io_service, short port);
  enum class HandleAcceptFlag { START, OK, ERROR };

private:
  void start_accept();
  void handle_accept(session* new_session,
      const boost::system::error_code& error);

  boost::asio::io_service& io_service_;
  tcp::acceptor acceptor_;
  HandleAcceptFlag flag = HandleAcceptFlag::START;
};

#endif
