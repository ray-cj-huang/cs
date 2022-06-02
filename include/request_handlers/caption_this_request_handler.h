#ifndef caption_this_request_handler_h
#define caption_this_request_handler_h

#include <boost/beast/http.hpp>
#include <string>
#include <vector>

#include "logger.h"
#include "request_handler.h"
#include "file_system_base.h"
#include "caption_file_parser.h"

namespace beast = boost::beast;
namespace http = beast::http;

class caption_this_request_handler: public request_handler
{
    friend class RequestHandlerTest;
    public:
        caption_this_request_handler(std::string location, std::string url, std::string root, 
                                     FileSystem* fs, caption_file_parser* cfp);
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
            int id,
            http::response<http::buffer_body>& response
        );
        status gallery( // GET gallery
            const boost::filesystem::path& path, 
            const http::request<http::string_body>& request, 
            http::response<http::buffer_body>& response
        ) const;
        std::string root_;
        FileSystem* fs_; // used for primitive dependency injection
        caption_file_parser* cfp_;
        std::vector<std::string> images_ = {
            "https://i.redd.it/nb0lmgrfrf821.png",
            "https://pagesix.com/wp-content/uploads/sites/3/2022/03/will-smith-chris-rock-24.jpg?quality=90&strip=all",
            "https://assets.vogue.com/photos/5891464f97a3db337a2494ed/master/w_2560%2Cc_limit/00-holding-best-barack-obama-memes.jpg",
            "https://honda-tech.com/forums/attachments/general-discussion-debate-40/106328d1265997440-memeable-picture-n729670114_2154820_5123.jpg",
            "https://cdn.hswstatic.com/gif/now-13eae70d-df40-43e8-bf2e-c0432a0c7264-1210-680.jpg",
            "https://parsi-times.com/wp-content/uploads/2019/10/caption-this.jpg",
            "https://cdn.realsport101.com/images/ncavvykf/epicstream/9f61396712ba4244e029d0646e1420fdea90567b-1277x716.jpg?rect=2,0,1272,716&w=700&h=394&dpr=2",
            "/static/team-bonding.jpg",
            "/static/howlin-ray.jpg",
            "/static/pampas.jpg",
            "/static/selfie.jpg",
            "https://qph.fs.quoracdn.net/main-qimg-3ea76d34e2cb9ff1d68375ce0aac95b3-lq",
            "https://i.kym-cdn.com/photos/images/facebook/002/305/349/1a2.png",

        };
};

#endif
