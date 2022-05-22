#ifndef request_handler_h
#define request_handler_h

#include <boost/beast/http.hpp>

#include "gtest/gtest.h"
#include "logger.h"

namespace beast = boost::beast;
namespace http = beast::http;

struct status {
    bool success;
    std::string err;
};

class request_handler {
public:
    request_handler(std::string location, std::string url);
    virtual status serve(char* req_data, size_t bytes_transferred, http::response<http::buffer_body> &res) = 0;
protected:
    void logRequest(http::status res_status) const;
    std::string location_;
    std::string url_;
private:
    friend class RequestHandlerTest;
    friend class RequestHandlerFactoryTest;
};

#endif
