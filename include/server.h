#ifndef server_h
#define server_h

#include <boost/asio.hpp>
#include "session.h"
#include "logger.h"
#include <unordered_set>
#include <unordered_map>

using boost::asio::ip::tcp;

class server
{
  friend class ServerTest;
public:
  server(boost::asio::io_service& io_service, short port, std::unordered_map<std::string, std::string> &static_paths, std::unordered_set<std::string> &echo_paths);
  enum class HandleAcceptFlag { START, OK, ERROR };

private:
  void start_accept();
  void handle_accept(session* new_session,
      const boost::system::error_code& error);

  boost::asio::io_service& io_service_;
  tcp::acceptor acceptor_;
  HandleAcceptFlag flag = HandleAcceptFlag::START;
  std::unordered_map<std::string, std::string> static_paths_;
  std::unordered_set<std::string> echo_paths_;
};

#endif
