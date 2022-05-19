#include <chrono>
#include <thread>
#include "sleep_request_handler.h"
#include "logger.h"

sleep_request_handler::sleep_request_handler(
    std::string location, std::string url)
    : request_handler(location, url)
{
}

status sleep_request_handler::serve(char* req_data, size_t bytes_transferred, http::response<http::buffer_body> &res) {
    res.result(http::status::ok);
    res.set(http::field::content_type, "text/plain");

    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_request_handler::SLEEP_DURATION));
    std::thread::id tid = std::this_thread::get_id();
    std::string data_string = "Thread slept for "+std::to_string(sleep_request_handler::SLEEP_DURATION/1000)+" seconds.";
    
    auto buf = new char[data_string.size()];
    memcpy(buf, data_string.c_str(), data_string.size());
    res.body().data = buf;
    res.body().size = data_string.size();
    Logger::logInfo("sleep_request_handler - serve - success");
    return {true, ""};
}
