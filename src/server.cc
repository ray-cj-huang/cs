#include "server.h"
#include "logger.h"

server::server(boost::asio::io_service& io_service, short port, std::unordered_map<std::string, std::string> &static_paths, std::unordered_set<std::string> &echo_paths)
    : io_service_(io_service),
      acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
      static_paths_(static_paths),
      echo_paths_(echo_paths)
{
  start_accept();
}

void server::start_accept()
{
  session* new_session = new session(io_service_, static_paths_, echo_paths_);
  acceptor_.async_accept(new_session->socket(),
      boost::bind(&server::handle_accept, this, new_session,
        boost::asio::placeholders::error));
  Logger::logInfo("Server - Start Accept");
}

void server::handle_accept(session* new_session,
      const boost::system::error_code& error)
{
  if (!error)
  {
    Logger::logInfo("Server - Handle Accept: Success");
    new_session->start();
    flag = HandleAcceptFlag::OK;
  }
  else
  {
    Logger::logError("Server - Handle Accept: Failed");
    delete new_session;
    flag = HandleAcceptFlag::ERROR;
  }

  start_accept();
}
