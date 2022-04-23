//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <signal.h>
#include <iostream>
#include <boost/bind.hpp>

#include "server.h"
#include "config_parser.h"
#include "logger.h"

using boost::asio::ip::tcp;

int main(int argc, char* argv[]) {
  try {
    Logger::init_logging();
    // Use NginxConfigParser to parse a config file,
    // then open a connection to the port in the config.
    NginxConfigParser parser;
    NginxConfig config;

    if (!parser.Parse(argv[1], &config)) {
      return -1;
    }

    int portNum = config.GetPort();
    if (portNum == -1) {
      return -1;
    }

    boost::asio::io_service io_service;

    using namespace std; // For atoi.
    server s(io_service, static_cast<short>(portNum));
    Logger::logInfo("Server Initalized.");

    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
