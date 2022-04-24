#ifndef request_handler_h
#define request_handler_h

#include <boost/beast/http.hpp>

#include "gtest/gtest.h"
#include "logger.h"

namespace beast = boost::beast;
namespace http = beast::http;

class request_handler
{
    friend class RequestHandlerTest;
    public:
        void put_data(char* req_data, size_t bytes_transferred);
        virtual void write_response(
            http::response<http::buffer_body> &res) = 0;

    protected:
        char* req_data_;
        size_t req_size_;
};

#endif
