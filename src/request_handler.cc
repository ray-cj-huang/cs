#include "request_handler.h"
#include "logger.h"

void request_handler::put_data(char* req_data,
                                size_t bytes_transferred)
{
    req_data_ = req_data;
    req_size_ = bytes_transferred;
    Logger::logInfo("request_handler - put_data - success");
}
