#include <boost/beast/http.hpp>

#include "echo_request_handler.h"
#include "logger.h"

namespace beast = boost::beast;
namespace http = beast::http;

echo_request_handler::echo_request_handler(
    std::string location, std::string url)
    : request_handler(location, url)
{
}

status echo_request_handler::serve(char* req_data, size_t bytes_transferred, http::response<http::buffer_body> &res) {
    res.result(http::status::ok);
    res.set(http::field::content_type, "text/plain");
    res.body().data = req_data;
    res.body().size = bytes_transferred;
    Logger::logInfo("echo_request_handler - serve - success");
    return {true, ""};
}
