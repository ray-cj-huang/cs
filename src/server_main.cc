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
#include <string>
#include <iterator>
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

    // find the endpoints for handling requests
    config.GetPaths(config.static_paths_, config.echo_paths_);

    std::string echo_paths = "", static_paths = "";
    for (const auto& elem: config.echo_paths_)
    {
        echo_paths += "\"" + elem + "\", ";
    }
    for (const auto& elem: config.static_paths_)
    {
        static_paths += "{url: \"" + elem.first +
                        "\", filepath: \"" + elem.second + "\"}, ";
    }

    boost::asio::io_service io_service;

    using namespace std; // For atoi.
    server s(io_service, static_cast<short>(portNum), config.static_paths_, config.echo_paths_);
    Logger::logInfo("Server Initalized.");
    Logger::logInfo("Port: " + std::to_string(portNum));
    Logger::logInfo("Echo Paths: " + echo_paths);
    Logger::logInfo("Static Paths: " + static_paths);
    io_service.run();
  }
  catch (std::exception& e)
  {
    std::string error_msg = "Exception";
    error_msg += e.what(); 
    Logger::logError(error_msg);
  }
  Logger::logInfo("Server Terminated.");
  return 0;
}
