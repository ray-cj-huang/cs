#include "crud_request_handler.h"
#include "file_system_base.h"

namespace beast = boost::beast;
namespace http = beast::http;

crud_request_handler::crud_request_handler(std::string location, std::string url, std::string root, FileSystem* fs):
    request_handler(location, url),
    root_(root),
    fs_(fs)
{}

status crud_request_handler::serve(char* req_data, size_t bytes_transferred, http::response<http::buffer_body> &res) {
    // TODO: echo handler in skeleton
    res.result(http::status::ok);
    res.set(http::field::content_type, "text/plain");
    res.body().data = req_data;
    res.body().size = bytes_transferred;
    Logger::logInfo("crud_request_handler - serve - success");
    return {true, ""};
}

http::status crud_request_handler::create(
    const boost::filesystem::path& path, 
    const http::request<http::string_body>& request
) const {
    // TODO: helper private function to create instance according to request body
    return http::status::ok;
}

http::status crud_request_handler::retrieve(
    const boost::filesystem::path& path,
    http::response<http::dynamic_body>& response
) const {
    // TODO: helper private function to retrieve instance content
    return http::status::ok;
}

http::status crud_request_handler::update(
    const boost::filesystem::path& path, 
    const http::request<http::string_body>& request, 
    http::response<http::dynamic_body>& response
) const {
    // TODO: helper private function to update existing instance content according to request body
    return http::status::ok;
}

http::status crud_request_handler::remove( // keyword 'delete' cannot be customized
    const boost::filesystem::path& path, 
    const http::request<http::string_body>& request, 
    http::response<http::dynamic_body>& response
) const {
    // TODO: helper private function to delete instance
    return http::status::ok;
}

http::status crud_request_handler::list(
    const boost::filesystem::path& path, 
    const http::request<http::string_body>& request, 
    http::response<http::dynamic_body>& response
) const {
    // TODO: helper private function to list all instances of an entity
    return http::status::ok;
}