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
#include <unordered_map>
#include <boost/bind.hpp>
#include <mutex>

#include "server.h"
#include "config_parser.h"
#include "request_handler_factory.h"
#include "error_request_handler_factory.h"
#include "echo_request_handler_factory.h"
#include "static_request_handler_factory.h"
#include "crud_request_handler_factory.h"
#include "health_request_handler_factory.h"
#include "sleep_request_handler_factory.h"
#include "logger.h"

#include "file_system_real.h"

using boost::asio::ip::tcp;

std::mutex mutex_fs; // for filesystem multithread race condition safety

// GCOVR_EXCL_START
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
    config.GetPaths(
        config.static_paths_,
        config.echo_paths_,
        config.CRUD_paths_,
        config.health_paths_,
        config.sleep_paths_
    );

    std::unordered_map<std::string, request_handler_factory*> routes;
    // inserting 404 handler first ensures that its path won't be overwritten
    routes.insert({{"/", new error_request_handler_factory()}});

    std::string echo_paths = "", static_paths = "", CRUD_paths = "", health_paths = "", sleep_paths = "";
    for (const auto& elem: config.echo_paths_) // search for all echo handlers specified in config
    {
        echo_paths += "\"" + elem + "\", ";
        routes.insert({{elem, new echo_request_handler_factory()}});
    }
    for (const auto& elem: config.static_paths_) // search for all static handlers specified in config
    {
        static_paths += "{url: \"" + elem.first +
                        "\", filepath: \"" + elem.second + "\"}, ";
        routes.insert({{elem.first, new static_request_handler_factory(elem.second)}});
    }

    FileSystem* fs = new RealFileSystem(mutex_fs); // multithread-safe filesystem for CRUD handlers

    for (const auto& elem: config.CRUD_paths_) // search for all CRUD handlers specified in config
    {
        CRUD_paths += "{url: \"" + elem.first +
                        "\", filepath: \"" + elem.second + "\"}, ";
        routes.insert({{elem.first, new crud_request_handler_factory(elem.second, fs)}});
    }
    for (const auto& elem: config.sleep_paths_) // search for all sleep handlers specified in config
    {
        sleep_paths += "\"" + elem + "\", ";
        routes.insert({{elem, new sleep_request_handler_factory()}});
    }

    for (const auto& elem: config.health_paths_) // search for all echo handlers specified in config
    {
        health_paths += "\"" + elem + "\", ";
        routes.insert({{elem, new health_request_handler_factory()}});
    }

    boost::asio::io_service io_service;

    server s(io_service, static_cast<short>(portNum), routes);
    Logger::logInfo("Server Initalized.");
    Logger::logInfo("Port: " + std::to_string(portNum));
    Logger::logInfo("Echo Paths: " + echo_paths);
    Logger::logInfo("Static Paths: " + static_paths);
    Logger::logInfo("CRUD Paths: " + CRUD_paths);
    Logger::logInfo("Health Paths: " + health_paths);
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
// GCOVR_EXCL_STOP
