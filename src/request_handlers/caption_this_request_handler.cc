#include "caption_this_request_handler.h"
#include "file_system_base.h"
#include "logger.h"

#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace fs = boost::filesystem;

caption_this_request_handler::caption_this_request_handler(std::string location, std::string url, std::string root,
                                                           FileSystem* fs, caption_file_parser* cfp):
    request_handler(location, url),
    root_(root),
    fs_(fs),
    cfp_(cfp)
{}

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string caption_this_request_handler::path_cat(
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
int caption_this_request_handler::get_next_id (const boost::filesystem::path& path) const {
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

status caption_this_request_handler::serve(char* req_data, size_t bytes_transferred, http::response<http::buffer_body> &res) {
    http::request_parser<http::string_body> req_parser;
    req_parser.eager(true);
    boost::beast::error_code ec;
    std::string string_data(req_data);
    size_t n_bytes = req_parser.put(boost::asio::buffer(string_data), ec);

    auto verb = req_parser.get().method();
    std::string full_path(req_parser.get().target());
    std::string rel_path = full_path.substr(full_path.find(location_) + location_.length());
    std::string path = caption_this_request_handler::path_cat(root_, rel_path);

    if (verb == http::verb::post) {
        return post_submission(path, req_parser.get(), res);
    }
    else if (verb == http::verb::get) {
        if (rel_path == "" || rel_path == "/") {
            return gallery(path_cat(path, "/submit"), req_parser.get(), res);
        } else if (isdigit(path.at(path.length() - 1))) {
            return get_submission_with_id(path.at(path.length() - 1) - '0', res);
        } else if (rel_path == "/submit" || rel_path == "/submit/") {
            return get_submission_page(res);
        } else {
            return {false, "invalid get request to caption this handler"};
        }
    }
    // else if (verb == http::verb::put) {
    //   return update(path, req_parser.get(), res);
    // }
    // else if (verb == http::verb::delete_) {
    //   return remove(path, req_parser.get(), res);
    // }

    return {false, "cannot handle request"};
}

// Finds next available ID and writes the submission
status caption_this_request_handler::post_submission(
    const boost::filesystem::path& path, 
    const http::request<http::string_body>& request,
    http::response<http::buffer_body>& response
) const {
    // if the entity does not exist
    if (!fs_->is_directory(path)) {
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
    response.set(http::field::content_type, "application/json");
    std::string data_string = "{ \"id\": " + std::to_string(next_id) + " }";
    auto buf = new char[data_string.size()];
    memcpy(buf, data_string.c_str(), data_string.size());
    response.body().data = buf;
    response.body().size = data_string.size();
    Logger::logInfo("caption_this_request_handler - serve - success");
    logRequest(response.result());
    return {true, ""};
}


// Returns generated html page with caption overlaid with image
status caption_this_request_handler::get_submission_with_id(
    int id,
    http::response<http::buffer_body>& response
) {
    const std::string PAGE_404_PATH = "../static/404_error.html";
    const std::string HTML_TEMPLATE_PATH = "../static/specific_submission_page.html";
    bool default_404 = false;
    std::string message = "";
    std::string path = path_cat(path_cat(root_ , "submit"), std::to_string(id));
    
    std::string data;
    std::string html_template;
    std::string html_payload;
    if (!fs_->read(path, data) || !fs_->read(HTML_TEMPLATE_PATH, html_template)) {
        default_404 = true;
        Logger::logError("404 file not found: " + path + " or " + HTML_TEMPLATE_PATH + ". Serving error page instead.");
        if (!fs_->read(PAGE_404_PATH, data)) {
            data = "404 Not Found";
        }
        response.result(http::status::not_found);
        response.set(http::field::content_type, "text/html");
        message = "404 Error Page";
    } else {
        std::string img_placeholder = "[img]";
        std::string top_text_placeholder = "[top text]";
        std::string bottom_text_placeholder = "[bottom text]";

        caption_file cf = cfp_->read(data);

        // create payload from template
        html_payload = html_template.substr(0, html_template.find(img_placeholder));
        html_payload += cf.img_url;
        int img_placeholder_end = html_template.find(img_placeholder) + img_placeholder.size();
        html_payload += html_template.substr(
            img_placeholder_end,
            html_template.find(top_text_placeholder) - img_placeholder_end);
        html_payload += cf.top_caption;
        int top_text_placeholder_end = html_template.find(top_text_placeholder) + top_text_placeholder.size();
        html_payload += html_template.substr(
            top_text_placeholder_end,
            html_template.find(bottom_text_placeholder) - top_text_placeholder_end);
        html_payload += cf.bot_caption;
        html_payload += html_template.substr(html_template.find(bottom_text_placeholder) + bottom_text_placeholder.size());

        response.result(http::status::ok);
        response.set(http::field::content_type, "text/html");
        Logger::logInfo("caption_this_request_handler - serve - success");
    }

    std::cout << html_payload << std::endl;

    char* buffer;
    buffer = new char[html_payload.size()];
    memcpy(buffer, html_payload.c_str(), html_payload.size());
    response.body().data = buffer;
    response.body().size = html_payload.size();
    logRequest(response.result());
    return {!default_404, message};

}

// Returns the static html page that allows users to submit captions
status caption_this_request_handler::get_submission_page(
    http::response<http::buffer_body>& response
) const {
    const std::string PAGE_404_PATH = "../static/404_error.html";
    const std::string SUBMIT_HTML_PATH = "../static/submit_page.html";
    bool default_404 = false;
    std::string message = "";
    
    std::string data;
    if (!fs_->read(SUBMIT_HTML_PATH, data)) {
        default_404 = true;
        Logger::logError("404 file not found: " + SUBMIT_HTML_PATH + ". Serving error page instead.");
        if (!fs_->read(PAGE_404_PATH, data)) {
            data = "404 Not Found";
        }
        response.result(http::status::not_found);
        response.set(http::field::content_type, "text/html");
        message = "404 Error Page";
    } else {
        response.result(http::status::ok);
        response.set(http::field::content_type, "text/html");
        Logger::logInfo("caption_this_request_handler - serve - success");
    }

    char* buffer = new char[data.size()];
    memcpy(buffer, data.c_str(), data.size());
    response.body().data = buffer;
    response.body().size = data.size();
    logRequest(response.result());
    return {!default_404, message};

}

// Returns a generated html page with the gallery of images with captions overlaid
status caption_this_request_handler::gallery(
    const boost::filesystem::path& path, 
    const http::request<http::string_body>& request, 
    http::response<http::buffer_body>& response
) const {
    const std::string PAGE_404_PATH = "../static/404_error.html";
    bool default_404 = false;
    std::string message = "";

    std::string data;
    if (!fs_->list_directory(path, data)) {
        default_404 = true;
        Logger::logError("404 file not found: " + path.string() + ". Serving error page instead.");
        if (!fs_->read(PAGE_404_PATH, data)) {
            data = "404 Not Found";
        }
        response.result(http::status::not_found);
        response.set(http::field::content_type, "text/html");
        message = "404 Error Page";
    } else {
        response.result(http::status::ok);
        response.set(http::field::content_type, "application/json");
        Logger::logInfo("caption_this_request_handler - serve - success");
    }

    char* buffer;
    buffer = new char[data.size()];
    memcpy(buffer, data.c_str(), data.size());
    response.body().data = buffer;
    response.body().size = data.size();
    logRequest(response.result());
    return {!default_404, message};
}

// status caption_this_request_handler::update(
//     const boost::filesystem::path& path, 
//     const http::request<http::string_body>& request, 
//     http::response<http::buffer_body>& response
// ) const {
//   if (fs_->exists(path)) {
//     try {
//       bool flag = fs_->upload_file(path, request.body());
//       if (flag) {
//         response.result(http::status::ok);
//         response.set(http::field::content_type, "text/plain");
//         std::string data_string = "File: " + path.string() + " updated";
//         auto buf = new char[data_string.size()];
//         memcpy(buf, data_string.c_str(), data_string.size());
//         response.body().data = buf;
//         response.body().size = data_string.size();
//         Logger::logInfo("caption_this_request_handler - serve - success");
//         logRequest(response.result());
//         return {true, ""};
//       }
//       else { // if file cannot be updated
//         response.result(http::status::internal_server_error);
//         response.set(http::field::content_type, "text/plain");
//         std::string data_string = "";
//         auto buf = new char[data_string.size()];
//         memcpy(buf, data_string.c_str(), data_string.size());
//         response.body().data = buf;
//         response.body().size = data_string.size();
//         Logger::logInfo("caption_this_request_handler - serve - failed");
//         logRequest(response.result());
//         return {false, "internal server error"};
//       }
//     } catch (std::exception &e) {
//         std::cerr << "Exception: " << e.what() << "\n";
//         std::stringstream msg_stream;
//         msg_stream << "Exception: " << e.what();
//         std::string msg = msg_stream.str();
//         Logger::logError(msg);
//     }
//   }

//   // if the requested file does not exist 
//   response.result(http::status::bad_request);
//   response.set(http::field::content_type, "text/plain");
//   std::string data_string = "";
//   auto buf = new char[data_string.size()];
//   memcpy(buf, data_string.c_str(), data_string.size());
//   response.body().data = buf;
//   response.body().size = data_string.size();
//   Logger::logInfo("caption_this_request_handler - serve - failed");
//   logRequest(response.result());
//   return {false, "bad request"};
// }

// status caption_this_request_handler::remove( // keyword 'delete' cannot be customized
//     const boost::filesystem::path& path, 
//     const http::request<http::string_body>& request, 
//     http::response<http::buffer_body>& response
// ) const {
//   if (fs_->exists(path)) {
//     try {
//       bool flag = fs_->remove(path);
//       if (flag) {
//         if (fs_->exists(path)) {
//           response.result(http::status::internal_server_error);
//           response.set(http::field::content_type, "text/plain");
//           std::string data_string = "";
//           auto buf = new char[data_string.size()];
//           memcpy(buf, data_string.c_str(), data_string.size());
//           response.body().data = buf;
//           response.body().size = data_string.size();
//           Logger::logInfo("caption_this_request_handler - serve - failed");
//           logRequest(response.result());
//           return {false, "internal server error"};
//         }

//         response.result(http::status::ok);
//         response.set(http::field::content_type, "text/plain");
//         std::string data_string = "File: " + path.string() + " deleted";
//         auto buf = new char[data_string.size()];
//         memcpy(buf, data_string.c_str(), data_string.size());
//         response.body().data = buf;
//         response.body().size = data_string.size();
//         Logger::logInfo("caption_this_request_handler - serve - success");
//         logRequest(response.result());
//         // check if the file still exist after deletion
//         return {true, ""};
//       }
//       else { // if error occurs when deleting the file
//         response.result(http::status::internal_server_error);
//         response.set(http::field::content_type, "text/plain");
//         std::string data_string = "";
//         auto buf = new char[data_string.size()];
//         memcpy(buf, data_string.c_str(), data_string.size());
//         response.body().data = buf;
//         response.body().size = data_string.size();
//         Logger::logInfo("caption_this_request_handler - serve - failed");
//         logRequest(response.result());
//         return {false, "internal server error"};
//       }
//     } catch (std::exception &e) {
//         std::cerr << "Exception: " << e.what() << "\n";
//         std::stringstream msg_stream;
//         msg_stream << "Exception: " << e.what();
//         std::string msg = msg_stream.str();
//         Logger::logError(msg);
//     }
//   }
//
//   // if the requested file does not exist 
//   response.result(http::status::bad_request);
//   response.set(http::field::content_type, "text/plain");
//   std::string data_string = "";
//   auto buf = new char[data_string.size()];
//   memcpy(buf, data_string.c_str(), data_string.size());
//   response.body().data = buf;
//   response.body().size = data_string.size();
//   Logger::logInfo("caption_this_request_handler - serve - failed");
//   logRequest(response.result());
//   return {false, "bad request"};

// }
