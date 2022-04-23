#include "session.h"
#include "logger.h"

#include <cstddef>
#include <iostream>

session::session(boost::asio::io_service& io_service)
  : socket_(io_service)
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
    std::string temp_static = "/static";
    std::string temp_echo = "/echo";

    Logger::logInfo("Logging Data:");
    Logger::logInfo(data);

    session::ParseRequestType req_type = session::parse_request(data,
        temp_static, temp_echo);

    if (req_type == session::ParseRequestType::STATICTYPE)
    {
        Logger::logInfo("Session - Static request recieved.");
        // echo placeholders for now
        // TODO(james168ma) add static request functionality
        session::echo(data, bytes_transferred);
        // session::write_file(error, data, bytes_transferred, file_paths_);
    }
    else if (req_type == session::ParseRequestType::ECHOTYPE)
    {
        
        Logger::logInfo("Session - Echo request recieved.");
        session::echo(data, bytes_transferred);
        // res_.result(http::status::ok);
        // res_.set(http::field::content_type, "text/plain");
        // res_.body().data = data;
        // res_.body().size = bytes_transferred;
        // http::async_write(socket_,
        //     res_,
        //     boost::bind(&session::handle_write, this,
        //     boost::asio::placeholders::error));
    }
    else if (req_type == session::ParseRequestType::NONGET)
    {
        Logger::logInfo("Session - Non-GET request recieved.");
        // TODO(james168ma) handle non get requests
        session::echo(data, bytes_transferred);
    }
    else if (req_type == session::ParseRequestType::BADREQUEST)
    {
        Logger::logInfo("Session - Bad request recieved.");
        // TODO(james168ma) handle bad requests
        session::echo(data, bytes_transferred);
    }
    else
    {
        Logger::logError(
           "Session - Handle Read: Failed. Logically should not have gotten here.");
    }
  }
  else
  {
    Logger::logError("Session - Handle Read: Failed");
    delete this;
  }
}

void session::echo(char* data, size_t bytes_transferred) {
    res_.result(http::status::ok);
        res_.set(http::field::content_type, "text/plain");
        res_.body().data = data;
        res_.body().size = bytes_transferred;
        http::async_write(socket_,
            res_,
            boost::bind(&session::handle_write, this,
            boost::asio::placeholders::error));
}

session::ParseRequestType session::parse_request(char* data,
      std::string& static_path, std::string& echo_path)
{
    http::request_parser<http::string_body> req_parser;
    boost::beast::error_code ec;
    std::string string_data(data);
    Logger::logInfo("Session - parse_request: string_data:");
    Logger::logInfo(string_data);
    size_t n_bytes = req_parser.put(boost::asio::buffer(string_data), ec);
    if (!req_parser.is_done() || ec)
    { // probably a bad http request and not that it's too big for buffer
        return session::ParseRequestType::BADREQUEST;
    }
    if (req_parser.get().method_string().to_string() == session::GET)
    {
        std::string target = req_parser.get().target().to_string();
        if (target.size() >= static_path.size() &&
            target.substr(0, static_path.size()) == static_path)
        {
            return session::ParseRequestType::STATICTYPE;
        }
        else if (target.size() >= echo_path.size() &&
            target.substr(0, echo_path.size()) == echo_path)
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

void session::write_file(const boost::system::error_code& error,
    char* data, size_t bytes_transferred,
    std::unordered_set <std::string> file_paths)
{

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
