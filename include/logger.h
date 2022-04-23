#ifndef LOG_H
#define LOG_H

#include <iostream>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/sinks/text_file_backend.hpp>

#include <boost/log/attributes/attribute.hpp>
#include <boost/log/attributes/attribute_cast.hpp>
#include <boost/log/attributes/attribute_value.hpp>
#include <boost/log/attributes/clock.hpp>

#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>

namespace logging = boost::log;
namespace keywords = boost::log::keywords;
namespace attrs = boost::log::attributes;
namespace sinks = boost::log::sinks;

using boost::asio::ip::tcp;

class Logger {
public:
    Logger();

    static void logTrace(std::string err);
    static void logInfo(std::string err);
    static void logDebug(std::string err);
    static void logWarning(std::string err);
    static void logError(std::string err);
    static void logFatal(std::string err);

    static void init_logging();
};
#endif