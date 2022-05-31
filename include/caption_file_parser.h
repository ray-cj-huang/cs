#ifndef caption_file_parser_h
#define caption_file_parser_h

#include <string>

/* Caption files are plaintext files with the following format:

Line 1: Top line caption
Line 2: Bottom line caption
Line 3: Image URL or local path

*/

class caption_file {
    public:
        std::string top_caption;
        std::string bot_caption;
        std::string img_url;
};

class caption_file_parser {
    public:
        caption_file read(std::string data);
};

#endif