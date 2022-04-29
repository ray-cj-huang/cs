#ifndef session_h
#define session_h

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/beast/http.hpp>

#include "request_handler.h"
#include "echo_request_handler.h"
#include "static_request_handler.h"
#include "logger.h"

namespace beast = boost::beast;
namespace http = beast::http;
using boost::asio::ip::tcp;

class session
{
  friend class SessionTest;
public:
  session(boost::asio::io_service& io_service, 
          echo_request_handler* echo_request_handler,
          static_request_handler* static_request_handler);

  tcp::socket& socket();

  void start();

  enum class ParseRequestType { STATICTYPE, ECHOTYPE,
                                 NONGET, BADREQUEST };

private:
  void handle_read(const boost::system::error_code& error,
      char* data, size_t bytes_transferred);

  session::ParseRequestType parse_request(char* data);

  void handle_write(const boost::system::error_code& error);

  const std::string GET = "GET";

  tcp::socket socket_;
  http::response<http::buffer_body> res_;
  enum { max_length = 1024 };
  char data_[max_length];
  echo_request_handler* echo_request_handler_;
  static_request_handler* static_request_handler_;
};

#endif
