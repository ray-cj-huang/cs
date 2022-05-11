#ifndef session_h
#define session_h

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/beast/http.hpp>

#include "request_handler.h"
#include "error_request_handler.h"
#include "request_handler_factory.h"
#include "error_request_handler_factory.h"

#include "logger.h"

namespace beast = boost::beast;
namespace http = beast::http;
using boost::asio::ip::tcp;

class session
{
  friend class SessionTest;
public:
  session(boost::asio::io_service& io_service, 
          std::unordered_map<std::string, request_handler_factory*> routes);

  tcp::socket& socket();

  void start();

private:
  void handle_read(const boost::system::error_code& error,
      char* data, size_t bytes_transferred);

  std::string match(std::string target);

  void handle_write();

  const std::string GET = "GET";

  tcp::socket socket_;
  http::response<http::buffer_body> res_;
  enum { max_length = 1024 };
  char data_[max_length];
  std::unordered_map<std::string, request_handler_factory*> routes_;
};

#endif
