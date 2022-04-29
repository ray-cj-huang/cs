#ifndef echo_request_handler_h
#define echo_request_handler_h

#include <boost/beast/http.hpp>
#include <unordered_set>

#include "logger.h"
#include "request_handler.h"

namespace beast = boost::beast;
namespace http = beast::http;

class echo_request_handler: public request_handler
{
    friend class RequestHandlerTest;
    public:
        echo_request_handler(std::unordered_set<std::string> &echo_locations);

        bool path_exists(std::string path);

        void write_response(
            http::response<http::buffer_body> &res);

    private:
        std::unordered_set<std::string> &echo_locations_;
};

#endif
