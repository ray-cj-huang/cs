#include "crud_request_handler.h"
#include "file_system_base.h"
#include "logger.h"

namespace beast = boost::beast;
namespace http = beast::http;

crud_request_handler::crud_request_handler(std::string location, std::string url, std::string root, FileSystem* fs):
    request_handler(location, url),
    root_(root),
    fs_(fs)
{}

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string path_cat(
  beast::string_view base,
  beast::string_view path
) {
    if(base.empty()) {
      return std::string(path);
    }
    std::string result(base);
    char constexpr path_separator = '/';
    if(result.back() == path_separator) {
        result.resize(result.size() - 1);
    }
    result.push_back(path_separator);
    if (!path.empty() && path[0] == path_separator) {
      result.append(path.data() + 1, path.size() - 1);
    } 
    else {
      result.append(path.data(), path.size());
    }
  return result;
}

// return the smallest available id inside a directory
int crud_request_handler::get_next_id (const boost::filesystem::path& path) const {
    int next_id = 1;
    while (next_id < INT_MAX) {
        boost::filesystem::path p{path.string() + "/" + std::to_string(next_id)};
        if (!fs_->exists(p)) {
            return next_id;
        }
        else {
            next_id += 1;
        }
    }
    return next_id;
}

status crud_request_handler::serve(char* req_data, size_t bytes_transferred, http::response<http::buffer_body> &res) {
    http::request_parser<http::string_body> req_parser;
    req_parser.eager(true);
    boost::beast::error_code ec;
    std::string string_data(req_data);
    size_t n_bytes = req_parser.put(boost::asio::buffer(string_data), ec);

    auto verb = req_parser.get().method();

    if (verb == http::verb::post) {
        std::string full_path(req_parser.get().target());
        std::string rel_path = full_path.substr(full_path.find(location_) + location_.length());
        std::string path = path_cat(root_, rel_path);
        return create(path, req_parser.get(), res);
    }
    return {true, ""};
}

status crud_request_handler::create(
    const boost::filesystem::path& path, 
    const http::request<http::string_body>& request,
    http::response<http::buffer_body>& response
) const {
    // if the entity does not exist
    if (!fs_->exists(path)) {
        try {
            fs_->create_directories(path);
        }
        catch (std::exception &e) {
            std::cerr << "Exception: " << e.what() << "\n";
            std::stringstream msg_stream;
            msg_stream << "Exception: " << e.what();
            std::string msg = msg_stream.str();
            Logger::logError(msg);
        }
    }
    // if the entity exists
    int next_id = get_next_id(path);
    boost::filesystem::path p{path.string() + "/" + std::to_string(next_id)};
    fs_->upload_file(p, request.body());

    response.result(http::status::ok);
    response.set(http::field::content_type, "text/plain");
    std::string data_string = "{ \"id\": " + std::to_string(next_id) + " }";
    auto buf = new char[data_string.size()];
    memcpy(buf, data_string.c_str(), data_string.size());
    response.body().data = buf;
    response.body().size = data_string.size();
    return {true, ""};
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