#ifndef error_request_handler_h
#define error_request_handler_h

#include <boost/beast/http.hpp>
#include <fstream>

#include "logger.h"
#include "request_handler.h"

namespace beast = boost::beast;
namespace http = beast::http;

class error_request_handler: public request_handler
{
    friend class RequestHandlerTest;
    public:
        error_request_handler(std::string location, std::string url);
        status serve(char* req_data, size_t bytes_transferred, http::response<http::buffer_body> &res);
        const std::string HTML = ".html";
};

#endif
