#include "session.h"

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
        boost::asio::placeholders::bytes_transferred));
}

void session::handle_read(const boost::system::error_code& error,
      size_t bytes_transferred)
{
  if (!error)
  {
    //TODO(!): Add verification that HTTP request is properly formatted.
    res_.result(http::status::ok);
    res_.set(http::field::content_type, "text/plain");
    res_.body().data = data_;
    res_.body().size = bytes_transferred;
    http::async_write(socket_,
        res_,
        boost::bind(&session::handle_write, this,
          boost::asio::placeholders::error));
  }
  else
  {
    delete this;
  }
}

void session::handle_write(const boost::system::error_code& error)
{
  if (!error)
  {
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&session::handle_read, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }
  else
  {
    delete this;
  }
}
