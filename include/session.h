#ifndef session_h
#define session_h

#include <unordered_set>
#include <string>
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

  enum class ParseRequestType { STATICTYPE, ECHOTYPE,
                                 NONGET, BADREQUEST };

private:
  void handle_read(const boost::system::error_code& error,
      char* data, size_t bytes_transferred);

  session::ParseRequestType parse_request(char* data,
      std::string& static_path, std::string& echo_path);

  void echo(char* data, size_t bytes_transferred);
    
  void write_file(const boost::system::error_code& error,
      char* data, size_t bytes_transferred,
      std::unordered_set <std::string> file_paths);

  void handle_write(const boost::system::error_code& error);

  const std::string GET = "GET";

  tcp::socket socket_;
  http::response<http::buffer_body> res_;
  enum { max_length = 1024 };
  char data_[max_length];
  std::unordered_set <std::string> file_paths_;
};

#endif
