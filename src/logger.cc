#include "logger.h"

namespace logging = boost::log;
namespace keywords = boost::log::keywords;
namespace attrs = boost::log::attributes;

using boost::asio::ip::tcp;

Logger::Logger() {
    init_logging();
    logging::add_console_log(std::cout, boost::log::keywords::format = ">> %Message%");
}

void Logger::init_logging() {
    logging::register_simple_formatter_factory<logging::trivial::severity_level, char>("Severity");

    logging::add_file_log(
        keywords::file_name = "../log/SYSLOG_%N.log",
        keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%] [%ProcessID%] [%LineID%] #[%NumOfLogs%] %Message%",
        keywords::rotation_size = 10 * 1024 * 1024,
        keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 0),
        keywords::auto_flush = true
    );

    logging::core::get()->set_filter
    (
        logging::trivial::severity >= logging::trivial::info
    );

    logging::core::get()->add_global_attribute("NumOfLogs", attrs::counter<int>(0, 1));

    logging::add_common_attributes();
}

void Logger::logTrace(std::string err){ BOOST_LOG_TRIVIAL(trace) << "Trace:  " << err; }
void Logger::logInfo(std::string err){ BOOST_LOG_TRIVIAL(info) << "Info: " << err; }
void Logger::logDebug(std::string err){ BOOST_LOG_TRIVIAL(debug) << "Debug: " << err; }
void Logger::logWarning(std::string err){ BOOST_LOG_TRIVIAL(warning) << "Warning: " << err; }
void Logger::logError(std::string err){ BOOST_LOG_TRIVIAL(error) << "Error: " << err; }
void Logger::logFatal(std::string err){ BOOST_LOG_TRIVIAL(fatal) << "Fatal: " << err; }

Logger* Logger::getLogger() {
    if (Logger::logger == 0)
        Logger::logger = new Logger();
    return Logger::logger;
}

Logger* Logger::logger = 0;