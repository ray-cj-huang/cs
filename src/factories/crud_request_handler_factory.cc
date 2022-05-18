#include "crud_request_handler_factory.h"

class FileSystem;

crud_request_handler_factory::crud_request_handler_factory(std::string root, FileSystem* fs)
    : root_(root), fs_(fs)
{  
}

request_handler* crud_request_handler_factory::create(
    std::string location, std::string url) 
{
    return new crud_request_handler(location, url, root_, fs_);
}
