#include "health_request_handler_factory.h"
#include "health_request_handler.h"

request_handler* health_request_handler_factory::create(
    std::string location, std::string url) 
{
    return new health_request_handler(location, url);
}
