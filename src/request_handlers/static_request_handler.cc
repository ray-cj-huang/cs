#include <boost/asio.hpp>
#include <boost/beast/http.hpp>
#include <exception>
#include <fstream>
#include <iostream>

#include "static_request_handler.h"
#include "logger.h"

namespace beast = boost::beast;
namespace http = beast::http;

static_request_handler::static_request_handler(
    std::string location,
    std::string url,
    std::string root)
    : request_handler(location, url),
      root_(root)
{
}

status static_request_handler::serve(char* req_data, size_t bytes_transferred, http::response<http::buffer_body> &res) {
    const std::string PAGE_404_PATH = "../static/404_error.html";
    http::request_parser<http::string_body> req_parser;
    boost::beast::error_code ec;
    std::string string_data(req_data);
    size_t n_bytes = req_parser.put(boost::asio::buffer(string_data), ec);

    std::string target = req_parser.get().target().to_string();
    Logger::logInfo("static_request_handler - received target " + target);
    
    size_t first_slash = target.find("/", 1);
    // this line ensures that static requests without a '/' do not crash the server 
    // by attempting to use string::npos in the substr() method.
    std::string complete_filepath = first_slash == std::string::npos ? 
        PAGE_404_PATH : root_ + target.substr(first_slash, target.size() - first_slash);
    
    Logger::logInfo("attempting to serve file at complete filepath " + complete_filepath);
    std::ifstream file(complete_filepath);
    
    int length = 0;
    char* buffer;
    bool default_404 = false;

    if (!file.is_open()) {
        default_404 = true;
        Logger::logError("404 file not found. Serving error page instead.");
        file.open(PAGE_404_PATH);
    }

    file.seekg(0, file.end);
    length = file.tellg();
    file.seekg(0, file.beg);
    buffer = new char[length];
    file.read(buffer, length);

    file.close();

    int period_loc = target.find_last_of(".");

    if (default_404) {
        res.result(http::status::not_found);
        std::string message = "404 Error Page";
        res.set(http::field::content_type, "text/html");
        res.body().data = buffer;
        res.body().size = length;
        return {false, message};
    }

    // no extension defaults to unsupported
    std::string extension = period_loc == std::string::npos ?
        "" : target.substr(period_loc, target.size()-period_loc);

    res.result(http::status::ok);

    if (extension == static_request_handler::TXT)
    {
        res.set(http::field::content_type, "text/plain");
        Logger::logInfo("static_request_handler - serve - .txt extension");
    }
    else if (extension == static_request_handler::HTML)
    {
        res.set(http::field::content_type, "text/html");
        Logger::logInfo("static_request_handler - serve - .html extension");
    }
    else if (extension == static_request_handler::JPG ||
              extension == static_request_handler::JPEG)
    {
        res.set(http::field::content_type, "image/jpeg");
        Logger::logInfo("static_request_handler - serve - .jpg extension");
    }
    else if (extension == static_request_handler::ZIP)
    {
        res.set(http::field::content_type, "application/zip");
        Logger::logInfo("static_request_handler - serve - .zip extension");
    }
    else
    {   // unsupported extension -> default to text/plain
        res.set(http::field::content_type, "text/plain");
        Logger::logInfo(
            "static_request_handler - serve -" \
            "unsupported extension, defaulting to text/plain");
    }

    res.body().data = buffer;
    res.body().size = length;
    Logger::logInfo("static_request_handler - serve - success");
    logRequest(res.result());
    return {true, ""};
}
