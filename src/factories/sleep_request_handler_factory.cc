#include "sleep_request_handler_factory.h"

request_handler* sleep_request_handler_factory::create(
    std::string location, std::string url) 
{
    return new sleep_request_handler(location, url);
}
