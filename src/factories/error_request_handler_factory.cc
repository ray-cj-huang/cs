#include "error_request_handler_factory.h"

request_handler* error_request_handler_factory::create(
    std::string location, std::string url) 
{
    return new error_request_handler(location, url);
}
