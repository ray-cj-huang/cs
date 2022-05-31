#include <string>
#include "caption_file_parser.h"
#include "logger.h"

caption_file caption_file_parser::read(std::string data) {
    caption_file res;
    try {
        res.top_caption = data.substr(0, data.find('\n'));
        std::string rest = data.substr(res.top_caption.size() + 1);
        res.bot_caption = rest.substr(0, rest.find('\n'));
        rest = rest.substr(res.bot_caption.size() + 1);
        res.img_url = rest.substr(0, rest.find('\n'));
        return res;
    }
    catch (std::exception &e) {
        std::stringstream msg_stream;
        msg_stream << "Error parsing data: " << e.what();
        std::string msg = msg_stream.str();
        Logger::logError(msg);
        return res;
    }
}