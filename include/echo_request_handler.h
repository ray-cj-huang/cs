#ifndef echo_request_handler_h
#define echo_request_handler_h

#include <boost/beast/http.hpp>

#include "logger.h"
#include "request_handler.h"

namespace beast = boost::beast;
namespace http = beast::http;

class echo_request_handler: public request_handler
{
    friend class RequestHandlerTest;
    public:
        echo_request_handler(std::string location, std::string url);
        void write_response(
            http::response<http::buffer_body> &res);
};

#endif
