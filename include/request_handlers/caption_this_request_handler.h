#ifndef caption_this_request_handler_h
#define caption_this_request_handler_h

#include <boost/beast/http.hpp>
#include <string>

#include "logger.h"
#include "request_handler.h"
#include "file_system_base.h"

namespace beast = boost::beast;
namespace http = beast::http;

class caption_this_request_handler: public request_handler
{
    friend class RequestHandlerTest;
    public:
        caption_this_request_handler(std::string location, std::string url, std::string root, FileSystem* fs);
        status serve(char* req_data, size_t bytes_transferred, http::response<http::buffer_body> &res) override;

    private:
        status get_submission_page( // GET submit
            http::response<http::buffer_body>& response            
        ) const;
        status post_submission( // POST submit
            const boost::filesystem::path& path, 
            const http::request<http::string_body>& request,
            http::response<http::buffer_body>& response
        ) const;
        status get_submission_with_id( // GET submit id
            const boost::filesystem::path& path,
            http::response<http::buffer_body>& response
        ) const;
        // status update( // POST(or PUT) submit existing ID
        //     const boost::filesystem::path& path, 
        //     const http::request<http::string_body>& request, 
        //     http::response<http::buffer_body>& response
        // ) const;
        // status remove( // DELETE existing ID
        //     const boost::filesystem::path& path, 
        //     const http::request<http::string_body>& request, 
        //     http::response<http::buffer_body>& response
        // ) const;
        status gallery( // GET gallery
            const boost::filesystem::path& path, 
            const http::request<http::string_body>& request, 
            http::response<http::buffer_body>& response
        ) const;
        int get_next_id(const boost::filesystem::path& path) const;
        std::string path_cat(beast::string_view base, beast::string_view path);
        std::string root_;
        FileSystem* fs_; // used for primitive dependency injection
};

#endif
