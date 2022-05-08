#include "echo_request_handler_factory.h"

request_handler* echo_request_handler_factory::create(
    std::string location, std::string url) 
{
    return new echo_request_handler(location, url);
}


