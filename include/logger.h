#ifndef LOG_H
#define LOG_H

#include <iostream>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>

#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>

namespace logging = boost::log;
namespace keywords = boost::log::keywords;
namespace attrs = boost::log::attributes;

using boost::asio::ip::tcp;

class Logger {
public:
    Logger();
    void init_logging();

    static void logTrace(std::string err);
    static void logInfo(std::string err);
    static void logDebug(std::string err);
    static void logWarning(std::string err);
    static void logError(std::string err);
    static void logFatal(std::string err);

    static Logger* getLogger();
private:
    static Logger* logger;
};
#endif