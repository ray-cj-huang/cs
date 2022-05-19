// Implementation based off https://www.boost.org/doc/libs/1_66_0/doc/html/boost_asio/example/cpp03/http/server3/server.cpp

#include "server.h"
#include "logger.h"
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

server::server(boost::asio::io_service& io_service, short port, 
               std::unordered_map<std::string, request_handler_factory*> routes, size_t thread_pool_size)
    : io_service_(io_service),
      acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
      routes_(routes),
      thread_pool_size_(thread_pool_size)
{
  start_accept();
}

void server::run()
{
  // Create a pool of threads to run all of the io_services.
  for (std::size_t i = 0; i < thread_pool_size_; ++i)
  {
    boost::shared_ptr<boost::thread> thread(new boost::thread(
          boost::bind(&boost::asio::io_context::run, &io_service_)));
    threads_.push_back(thread);
  }

  Logger::logInfo("Server generated a thread pool of size " + std::to_string(threads_.size()));

  // Wait for all threads in the pool to exit.
  for (std::size_t i = 0; i < threads_.size(); ++i)
    threads_[i]->join();
}

void server::start_accept()
{
  session* new_session = new session(io_service_, routes_);
  Logger::logInfo("Server - ready to accept new connections.");
  acceptor_.async_accept(new_session->socket(),
      boost::bind(&server::handle_accept, this, new_session,
        boost::asio::placeholders::error));
}

void server::handle_accept(session* new_session,
      const boost::system::error_code& error)
{
  if (!error)
  {
    Logger::logInfo("Server - accepted connection.");
    new_session->start();
    flag = HandleAcceptFlag::OK;
  }
  else
  {
    Logger::logError("Server - failed to start new connection.");
    delete new_session;
    flag = HandleAcceptFlag::ERROR;
  }

  start_accept();
}
