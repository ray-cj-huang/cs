#ifndef static_request_handler_h
#define static_request_handler_h

#include <boost/beast/http.hpp>
#include <string>
#include <unordered_map>

#include "logger.h"
#include "request_handler.h"

namespace beast = boost::beast;
namespace http = beast::http;

class static_request_handler: public request_handler
{
    friend class RequestHandlerTest;
    public:
        static_request_handler(
            std::unordered_map <std::string, std::string> &static_locations);

        void write_response(
            http::response<http::buffer_body> &res);

        const std::string TXT = ".txt";
        const std::string HTML = ".html";
        const std::string JPG = ".jpg";
        const std::string JPEG = ".jpeg";
        const std::string ZIP = ".zip";
    
    private:
        std::unordered_map <std::string, std::string> &static_locations_;
};

#endif
