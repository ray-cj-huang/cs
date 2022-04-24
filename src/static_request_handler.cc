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
    std::unordered_map <std::string, std::string> &static_locations)
    : static_locations_(static_locations)
{
}

void static_request_handler::write_response(
        http::response<http::buffer_body> &res)
{   
    http::request_parser<http::string_body> req_parser;
    boost::beast::error_code ec;
    std::string string_data(req_data_);
    size_t n_bytes = req_parser.put(boost::asio::buffer(string_data), ec);

    std::string target = req_parser.get().target().to_string();
    int first_slash = target.find("/", 1);
    std::string static_path = target.substr(0, first_slash);
    auto static_folder_pair = static_locations_.find(static_path);

    if (static_folder_pair != static_locations_.end())
    {
        std::string complete_filepath = static_folder_pair->second +
            target.substr(first_slash, target.size() - first_slash);
        std::ifstream file;
        int length = 0;
        char* buffer;
        try
        {
            file.open(complete_filepath);
            file.seekg(0, file.end);
            length = file.tellg();
            file.seekg(0, file.beg);
            buffer = new char[length];
            file.read(buffer, length);
        }
        catch (const std::exception& e)
        {
            res.result(http::status::not_found);
            return;
        }
        file.close();

        int period_loc = target.find_last_of(".");

        // no extension defaults to unsupported
        std::string extension = period_loc == std::string::npos ?
            "" : target.substr(period_loc, target.size()-period_loc);

        if (extension == static_request_handler::TXT)
        {
            res.set(http::field::content_type, "text/plain");
            Logger::logInfo("echo_request_handler - write_response - .txt extension");
        }
        else if (extension == static_request_handler::HTML)
        {
            res.set(http::field::content_type, "text/html");
            Logger::logInfo("echo_request_handler - write_response - .html extension");
        }
        else if (extension == static_request_handler::JPG ||
                 extension == static_request_handler::JPEG)
        {
            res.set(http::field::content_type, "image/jpeg");
            Logger::logInfo("echo_request_handler - write_response - .jpg extension");
        }
        else if (extension == static_request_handler::ZIP)
        {
            res.set(http::field::content_type, "application/zip");
            Logger::logInfo("echo_request_handler - write_response - .zip extension");
        }
        else
        {   // unsupported extension -> default to text/plain
            res.set(http::field::content_type, "text/plain");
            Logger::logInfo(
                "echo_request_handler - write_response -" \
                "unsupported extension, defaulting to text/plain");
        }
        res.result(http::status::ok);
        res.body().data = buffer;
        res.body().size = length;
        Logger::logInfo("echo_request_handler - write_response - success");
    }
    else
    {   // technically should not get here
        res.result(http::status::bad_request);
        Logger::logError("echo_request_handler - write_response - should not have gotten here");
    }
}