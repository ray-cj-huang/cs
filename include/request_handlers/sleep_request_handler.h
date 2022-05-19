#ifndef sleep_request_handler_h
#define sleep_request_handler_h

#include <boost/beast/http.hpp>
#include "request_handler.h"

namespace beast = boost::beast;
namespace http = beast::http;

class sleep_request_handler: public request_handler
{
    friend class RequestHandlerTest;
    public:
        sleep_request_handler(std::string location, std::string url);
        status serve(char* req_data, size_t bytes_transferred, http::response<http::buffer_body> &res);

        const int SLEEP_DURATION = 5000; // 10 seconds
};

#endif
