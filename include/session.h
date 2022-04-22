#ifndef session_h
#define session_h

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/beast/http.hpp>

#include "logger.h"

namespace beast = boost::beast;
namespace http = beast::http;
using boost::asio::ip::tcp;

class session
{
  friend class SessionTest;
public:
  session(boost::asio::io_service& io_service);

  tcp::socket& socket();

  void start();

private:
  void handle_read(const boost::system::error_code& error,
      char* data, size_t bytes_transferred);

  void handle_write(const boost::system::error_code& error);

  tcp::socket socket_;
  http::response<http::buffer_body> res_;
  enum { max_length = 1024 };
  char data_[max_length];
};

#endif
