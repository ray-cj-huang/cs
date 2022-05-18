#include "request_handler.h"
#include "logger.h"

request_handler::request_handler(std::string location, std::string url)
    : location_(location),
      url_(url)
{
}
