#include "logger.h"

namespace logging = boost::log;
namespace keywords = boost::log::keywords;
namespace attrs = boost::log::attributes;
namespace sinks = boost::log::sinks;

using boost::asio::ip::tcp;

void Logger::init_logging() {
    logging::register_simple_formatter_factory<logging::trivial::severity_level, char>("Severity");

    logging::add_console_log(std::cout, boost::log::keywords::format = ">> %Message%\n");
    logging::add_file_log(
        keywords::file_name = "../log/SYSLOG.log",
        keywords::format = "[%LocalTimeStamp%] [%ThreadID%] [%Severity%] [%ProcessID%] [%LineID%] #[%NumOfLogs%] %Message%\n",
        keywords::rotation_size = 10 * 1024 * 1024,
        keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 0),
        keywords::auto_flush = true
    );

    logging::core::get()->set_filter
    (
        logging::trivial::severity >= logging::trivial::info
    );

    logging::core::get()->add_global_attribute("NumOfLogs", attrs::counter<int>(1, 1));
    logging::core::get()->add_global_attribute("LocalTimeStamp", attrs::local_clock());

    logging::add_common_attributes();
}

void Logger::logTrace(std::string err)      { BOOST_LOG_TRIVIAL(trace)      << "Trace:  "   << err; }
void Logger::logInfo(std::string err)       { BOOST_LOG_TRIVIAL(info)       << "Info: "     << err; }
void Logger::logDebug(std::string err)      { BOOST_LOG_TRIVIAL(debug)      << "Debug: "    << err; }
void Logger::logWarning(std::string err)    { BOOST_LOG_TRIVIAL(warning)    << "Warning: "  << err; }
void Logger::logError(std::string err)      { BOOST_LOG_TRIVIAL(error)      << "Error: "    << err; }
void Logger::logFatal(std::string err)      { BOOST_LOG_TRIVIAL(fatal)      << "Fatal: "    << err; }