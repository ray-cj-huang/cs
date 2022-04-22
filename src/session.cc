#include "session.h"
#include "logger.h"

session::session(boost::asio::io_service& io_service)
    : socket_(io_service)
{
}

tcp::socket& session::socket()
{
  return socket_;
}

void session::start()
{
  socket_.async_read_some(boost::asio::buffer(data_, max_length),
      boost::bind(&session::handle_read, this,
        boost::asio::placeholders::error,
        data_,
        boost::asio::placeholders::bytes_transferred));
}

void session::handle_read(const boost::system::error_code& error,
      char* data, size_t bytes_transferred)
{
  Logger* logger = Logger::getLogger();
  if (!error)
  {
    //TODO(!): Add verification that HTTP request is properly formatted.
    res_.result(http::status::ok);
    res_.set(http::field::content_type, "text/plain");
    res_.body().data = data;
    res_.body().size = bytes_transferred;
    http::async_write(socket_,
        res_,
        boost::bind(&session::handle_write, this,
          boost::asio::placeholders::error));
    logger->logInfo("Session - Handle Read: Success");
  }
  else
  {
    logger->logError("Session - Handle Read: Failed");
    delete this;
  }
}

void session::handle_write(const boost::system::error_code& error)
{
  Logger * logger = Logger::getLogger();
  if (!error)
  {
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&session::handle_read, this,
          boost::asio::placeholders::error,
          data_,
          boost::asio::placeholders::bytes_transferred));
    logger->logInfo("Session - Handle Write: Success");
  }
  else
  {
    logger->logError("Session - Handle Write: Failed");
    delete this;
  }
}
