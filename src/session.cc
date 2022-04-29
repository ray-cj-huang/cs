#include "session.h"
#include "logger.h"

#include <unordered_map>
#include <string>

session::session(boost::asio::io_service& io_service,
                 echo_request_handler* echo_request_handler,
                 static_request_handler* static_request_handler)
  : socket_(io_service),
    echo_request_handler_(echo_request_handler),
    static_request_handler_(static_request_handler)
{
}

tcp::socket& session::socket() {
  return socket_;
}

void session::start() {
  socket_.async_read_some(boost::asio::buffer(data_, max_length),
      boost::bind(&session::handle_read, this,
        boost::asio::placeholders::error,
        data_,
        boost::asio::placeholders::bytes_transferred));
  Logger::logInfo("Session - Start");
}

void session::handle_read(const boost::system::error_code& error,
      char* data, size_t bytes_transferred)
{
  if (!error)
  {

    if (socket_.is_open()) {
      Logger::logInfo("Logging Data from " + socket_.remote_endpoint().address().to_string());
    }
    Logger::logInfo(data);

    session::ParseRequestType req_type = session::parse_request(data);

    request_handler* req_handler;

    if (req_type == session::ParseRequestType::STATICTYPE)
    {
        Logger::logInfo("Session - Static request recieved.");
        req_handler = static_request_handler_;
    }
    else if (req_type == session::ParseRequestType::ECHOTYPE)
    {
        
        Logger::logInfo("Session - Echo request recieved.");
        req_handler = echo_request_handler_;
    }
    else if (req_type == session::ParseRequestType::NONGET)
    {
        Logger::logInfo("Session - Non-GET request recieved.");
        // TODO(!) handle non get requests
        req_handler = echo_request_handler_;
    }
    else if (req_type == session::ParseRequestType::BADREQUEST)
    {
        Logger::logInfo("Session - Bad request recieved.");
        // TODO(!) handle bad requests
        req_handler = echo_request_handler_;
    }
    else
    {
        Logger::logError(
           "Session - Handle Read: Failed. Logically should not have gotten here.");
    }

    req_handler->put_data(data, bytes_transferred);
    req_handler->write_response(res_);
    http::async_write(socket_,
            res_,
            boost::bind(&session::handle_write, this,
            boost::asio::placeholders::error));
    delete req_handler;
  }
  else
  {
    Logger::logError("Session - Handle Read: Failed");
    delete this;
  }
}

session::ParseRequestType session::parse_request(char* data)
{
    http::request_parser<http::string_body> req_parser;
    boost::beast::error_code ec;
    std::string string_data(data);
    size_t n_bytes = req_parser.put(boost::asio::buffer(string_data), ec);
    if (!req_parser.is_done() || ec)
    { // probably a bad http request and not that it's too big for buffer
        return session::ParseRequestType::BADREQUEST;
    }
    if (req_parser.get().method_string().to_string() == session::GET)
    {
        std::string target = req_parser.get().target().to_string();
        
        std::string path = target.substr(0, target.find("/", 1));

        if (static_request_handler_->path_exists(path))
        {
            return session::ParseRequestType::STATICTYPE;
        }
        else if (echo_request_handler_->path_exists(path))
        {
            return session::ParseRequestType::ECHOTYPE;
        }
        else
        {
            return session::ParseRequestType::BADREQUEST;
        }
    }
    Logger::logInfo("Session - parse_request: wrong method:");
    Logger::logInfo(req_parser.get().method_string().to_string());
    return session::ParseRequestType::NONGET;
}

void session::handle_write(const boost::system::error_code& error)
{
  if (!error)
  {
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&session::handle_read, this,
          boost::asio::placeholders::error,
          data_,
          boost::asio::placeholders::bytes_transferred));
    Logger::logInfo("Session - Handle Write: Success");
  } else {
    Logger::logInfo("Session - Handle Write: No more data to write. Closing session.");
    delete this;
  }
}
