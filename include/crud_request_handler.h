#ifndef crud_request_handler_h
#define crud_request_handler_h

#include <boost/beast/http.hpp>
#include <string>

#include "logger.h"
#include "request_handler.h"

namespace beast = boost::beast;
namespace http = beast::http;

class FileSystem;

class crud_request_handler: public request_handler
{
    friend class RequestHandlerTest;
    public:
        crud_request_handler(std::string location, std::string url, std::string root, FileSystem* fs);
        status serve(char* req_data, size_t bytes_transferred, http::response<http::buffer_body> &res) override;

    private:
        http::status create(
            const boost::filesystem::path& path, 
            const http::request<http::string_body>& request
        ) const;
        http::status retrieve(
            const boost::filesystem::path& path,
            http::response<http::dynamic_body>& response
        ) const;
        http::status update(
            const boost::filesystem::path& path, 
            const http::request<http::string_body>& request, 
            http::response<http::dynamic_body>& response
        ) const;
        http::status remove( // keyword 'delete' cannot be customized
            const boost::filesystem::path& path, 
            const http::request<http::string_body>& request, 
            http::response<http::dynamic_body>& response
        ) const;
        http::status list(
            const boost::filesystem::path& path, 
            const http::request<http::string_body>& request, 
            http::response<http::dynamic_body>& response
        ) const;

        std::string root_;
        FileSystem* fs_; // used for primitive dependency injection
};

#endif
