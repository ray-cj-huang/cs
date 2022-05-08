#include "request_handler.h"
#include "logger.h"

request_handler::request_handler(std::string location, std::string url)
    : location_(location),
      url_(url)
{
}

void request_handler::put_data(char* req_data,
                                size_t bytes_transferred)
{
    req_data_ = req_data;
    req_size_ = bytes_transferred;
    Logger::logInfo("request_handler - put_data - success");
}
