#include "caption_this_request_handler_factory.h"
#include "file_system_base.h"
#include "caption_file_parser.h"

#include <string>

caption_this_request_handler_factory::caption_this_request_handler_factory(std::string root, FileSystem* fs)
    : root_(root), fs_(fs)
{  
}

request_handler* caption_this_request_handler_factory::create(
    std::string location, std::string url) 
{
    return new caption_this_request_handler(location, url, root_, fs_, new caption_file_parser());
}
