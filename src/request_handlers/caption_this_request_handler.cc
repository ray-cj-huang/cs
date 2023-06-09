#include "caption_this_request_handler.h"
#include "file_system_base.h"
#include "logger.h"

#include <stdlib.h>
#include <string>
#include <vector>
#include <sstream>

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

bool isNumber(const std::string& str)
{
    if (str.length() == 0) return false;
    for (char const &c : str) {
        if (std::isdigit(c) == 0) return false;
    }
    return true;
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
    std::string path = fs_->path_cat(root_, rel_path);

    if (verb == http::verb::post && 
        (rel_path == "/submit" || rel_path == "/submit/")
    ) {
        return post_submission(path, req_parser.get(), res);
    }
    else if (verb == http::verb::get) {

        std::string last_item_in_path =
            (rel_path == "" || (rel_path.length() == 1 && rel_path[0] == '/'))
                ? ""
                : rel_path[rel_path.length() - 1] == '/'
                    ? rel_path.substr(rel_path.find_last_of("/") + 1,
                        rel_path.length() - rel_path.find_last_of("/") - 2)
                    : rel_path.substr(rel_path.find_last_of("/") + 1,
                        rel_path.length() - rel_path.find_last_of("/") - 1);

        if (rel_path == "" || rel_path == "/") {
            return gallery(fs_->path_cat(path, "/submit"), req_parser.get(), res);
        } else if (isNumber(last_item_in_path) &&
                    ( rel_path.substr(0, rel_path.find(last_item_in_path)) == "" ||
                      rel_path.substr(0, rel_path.find(last_item_in_path)) == "/" )
        ) {
            return get_submission_with_id(std::stoi(last_item_in_path), res);
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
    int next_id = fs_->get_next_id(path);
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
    std::string path = fs_->path_cat(fs_->path_cat(root_ , "submit"), std::to_string(id));
    
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

        data = html_payload;
        response.result(http::status::ok);
        response.set(http::field::content_type, "text/html");
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

// Returns the static html page that allows users to submit captions
status caption_this_request_handler::get_submission_page(
    http::response<http::buffer_body>& response
) const {
    const std::string PAGE_404_PATH = "../static/404_error.html";
    const std::string SUBMIT_HTML_PATH = "../static/submit_page.html";
    bool default_404 = false;
    std::string message = "";
    
    std::string data;
    std::string html_template;
    if (!fs_->read(SUBMIT_HTML_PATH, html_template)) {
        default_404 = true;
        Logger::logError("404 file not found: " + SUBMIT_HTML_PATH + ". Serving error page instead.");
        if (!fs_->read(PAGE_404_PATH, data)) {
            data = "404 Not Found";
        }
        response.result(http::status::not_found);
        response.set(http::field::content_type, "text/html");
        message = "404 Error Page";
    } else {
        int random_num = rand() % images_.size();
        std::string img_url = images_[random_num];
        std::string img_placeholder = "[img]";

        // create payload from template
        std::string html_payload = html_template.substr(0, html_template.find(img_placeholder));
        html_payload += img_url;
        int img_placeholder_end = html_template.find(img_placeholder) + img_placeholder.size();
        html_payload += html_template.substr(
            img_placeholder_end,
            html_template.substr(img_placeholder_end).find(img_placeholder));
        html_payload += img_url;
        int img_placeholder_end_2 = html_template.find(img_placeholder)
            + html_template.substr(img_placeholder_end).find(img_placeholder)
            + (img_placeholder.size() * 2);
        html_payload += html_template.substr(img_placeholder_end_2);

        data = html_payload;

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
    const std::string HTML_TEMPLATE_PATH =  "../static/gallery_page.html";
    bool default_404 = false;
    std::string message = "";
    std::string html_template = "";
    std::string placeholder = "PLACEHOLDER";


    std::string data;
    if (!fs_->list_paths_directory(path, data)) {
        default_404 = true;
        Logger::logError("404 file not found: " + path.string() + ". Serving error page instead.");
        if (!fs_->read(PAGE_404_PATH, data)) {
            data = "404 Not Found";
        }
        response.result(http::status::not_found);
        response.set(http::field::content_type, "text/html");
        message = "404 Error Page";
    } else {

        // split up the data string into a vector of different paths
        std::vector<std::string> result;
        std::stringstream s_stream(data);
        while(s_stream.good()) {
            std::string substr;
            getline(s_stream, substr, ',');
            result.push_back(substr);
        }
        for(int i = 0; i < result.size(); ++i) {
            std::cout << result.at(i) << std::endl;
        }

        // create a vector to store all the html divs we create
        std::vector<std::string> divs;

        bool paths_fully_processed = true;

        // for each path, generate a new div 
        for (int i = 0; i < result.size(); ++i) {
            std::string path = result[i];
            std::string content;

            if (!fs_->read(path, content)) {
                paths_fully_processed = false;
                break;
            }
            else {
                caption_file cf = cfp_->read(content);

                // generate a new div based on the content 
                std::string div =
                    "<button class=\"relative text-white text-sm flex flex-wrap md:w-1/3 w-full hover:opacity-70\"> \
                        <a href=\"/caption/" + std::to_string(i+1) + "\" class=\"w-full h-full p-1 md:p-2\"> \
                            <img src=\"" + cf.img_url  + "\"alt=\"Image\" class=\"block object-cover object-center w-full h-full rounded-lg\"> \
                            <div class=\"absolute left-1/2 transform -translate-x-1/2 top-3 items-center break-words max-w-[80%]\">" + cf.top_caption + "</div> \
                            <div class=\"absolute left-1/2 transform -translate-x-1/2 bottom-3 items-center break-words max-w-[80%]\">" + cf.bot_caption + "</div> \
                        </a> \
                    </button>";
                divs.push_back(div);
            }
        }

        // throw 404 if something unexpected happens
        if (!paths_fully_processed || !fs_->read(HTML_TEMPLATE_PATH, html_template)) {
            // throw 404 here, something went wrong
            default_404 = true;
            Logger::logError("404 file not found: " + path.string() + ". Serving error page instead.");
            if (!fs_->read(PAGE_404_PATH, data)) {
                data = "404 Not Found";
            }
            response.result(http::status::not_found);
            response.set(http::field::content_type, "text/html");
            message = "404 Error Page";
        }

        // replace the template part of the gallery with the divs in the divs vector
        std::string html_payload = html_template.substr(0, html_template.find(placeholder));
        for (int i = 0; i < divs.size(); ++i) {
            html_payload = html_payload + divs[i];
        }
        html_payload = html_payload + html_template.substr(html_template.find(placeholder) + placeholder.size(), html_template.size());
        data = html_payload;

        response.result(http::status::ok);
        response.set(http::field::content_type, "text/html");
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
