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
    std::string ERROR_400_PATH = "../static/400_error.html";
    std::string ERROR_404_PATH = "../static/404_error.html";

    http::request_parser<http::string_body> req_parser;
    boost::beast::error_code ec;
    std::string string_data(req_data);
    size_t n_bytes = req_parser.put(boost::asio::buffer(string_data), ec);

    std::string error_path;
    if (!req_parser.is_done() || ec)
    {
        Logger::logInfo("Serving 400 Error Page");
        error_path = ERROR_400_PATH;
        res.result(http::status::bad_request);
    }
    else
    {
        Logger::logInfo("Serving 404 Error Page");
        error_path = ERROR_404_PATH;
        res.result(http::status::not_found);
    }

    int length = 0;
    char* buffer;

    std::ifstream file(error_path);

    file.seekg(0, file.end);
    length = file.tellg();
    file.seekg(0, file.beg);
    buffer = new char[length];
    file.read(buffer, length);

    file.close();

    std::string extension;
    extension = error_request_handler::HTML;

    res.set(http::field::content_type, "text/html");
    res.body().data = buffer;
    res.body().size = length;

    Logger::logInfo("error_request_handler - serve - success");
    return {true, ""};
}
