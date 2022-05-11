#include <boost/beast/http.hpp>

#include "error_request_handler.h"
#include "logger.h"

namespace beast = boost::beast;
namespace http = beast::http;

error_request_handler::error_request_handler(
    std::string location, std::string url)
    : request_handler(location, url)
{
}

status error_request_handler::serve(char* req_data, size_t bytes_transferred, http::response<http::buffer_body> &res)
{
    Logger::logInfo("Serving 404 Error Page");
    std::string page_404_path = "../static/404_error.html";

    int length = 0;
    char* buffer;

    std::ifstream file(page_404_path);

    file.close();

    std::string extension;
    extension = error_request_handler::HTML;

    res.result(http::status::not_found);
    res.set(http::field::content_type, "text/html");
    res.body().data = buffer;
    res.body().size = length;

    Logger::logInfo("error_request_handler - serve - success");
    return {true, ""};
}
