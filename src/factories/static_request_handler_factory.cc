#include "static_request_handler_factory.h"

static_request_handler_factory::static_request_handler_factory(std::string root)
    : root_(root)
{  
}

request_handler* static_request_handler_factory::create(
    std::string location, std::string url) 
{
    return new static_request_handler(location, url, root_);
}
