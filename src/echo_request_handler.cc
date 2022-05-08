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

void echo_request_handler::write_response(
        http::response<http::buffer_body> &res)
{
    res.result(http::status::ok);
    res.set(http::field::content_type, "text/plain");
    res.body().data = req_data_;
    res.body().size = req_size_;
    Logger::logInfo("echo_request_handler - write_response - success");
}
