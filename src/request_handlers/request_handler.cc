#include "request_handler.h"
#include "logger.h"

namespace beast = boost::beast;
namespace http = beast::http;

request_handler::request_handler(std::string location, std::string url)
    : location_(location),
      url_(url)
{
}
// can also change this to just take in the res.result() directly
void request_handler::logRequest(http::status res_status) const {
    Logger::logInfo("METRICS - request_handler:" + location_);
    Logger::logInfo("METRICS - request_url:" + url_);
    std::string res_code = "";

    switch (res_status) {
      case http::status::bad_request:
        res_code = "400";
        break;
      case http::status::not_found:
        res_code = "404";
        break;
      case http::status::internal_server_error:
        res_code = "500";
        break;
      case http::status::ok:
      default:
        res_code = "200";
        break;
    }

    Logger::logInfo("METRICS - response_code:" + res_code);
}
