#include <boost/beast/http.hpp>
#include <string>

#include "health_request_handler.h"
#include "logger.h"

namespace beast = boost::beast;
namespace http = beast::http;

health_request_handler::health_request_handler(
    std::string location, std::string url)
    : request_handler(location, url)
{
}

status health_request_handler::serve(char* req_data, size_t bytes_transferred, http::response<http::buffer_body> &res) {
    res.result(http::status::ok);
    res.set(http::field::content_type, "text/plain");
    char* data = new char[3];
    sprintf(data, "OK");
    res.body().data = data;
    res.body().size = strlen(data);
    Logger::logInfo("health_request_handler - serve - success");
    return {true, ""};
}
