#include "session.h"
#include "request_handler.h"

session::session(boost::asio::io_service& io_service, std::unordered_map<std::string, request_handler_factory*> routes)
  : socket_(io_service),
    routes_(routes)
{
}

tcp::socket& session::socket() {
  return socket_;
}

void session::start() {
  socket_.async_read_some(boost::asio::buffer(data_, max_length),
      boost::bind(&session::handle_read, this,
        boost::asio::placeholders::error,
        data_,
        boost::asio::placeholders::bytes_transferred));
  Logger::logInfo("Session - Start");
}

void session::handle_read(const boost::system::error_code& error,
      char* data, size_t bytes_transferred)
{
  if (!error)
  {

    if (socket_.is_open()) {
      Logger::logInfo("Logging Data from " + socket_.remote_endpoint().address().to_string());
    }
    Logger::logInfo(data);

    request_handler_factory* factory;
    request_handler* req_handler;

    http::request_parser<http::string_body> req_parser;
    boost::beast::error_code ec;
    std::string string_data(data);
    size_t n_bytes = req_parser.put(boost::asio::buffer(string_data), ec);

    if (!req_parser.is_done() || ec)
    { // probably a bad http request and not that it's too big for buffer
        Logger::logError("Session - bad HTTP request.");
        return;
    }
    else if (req_parser.get().method_string().to_string() != session::GET)
    {
        Logger::logError("Session - method " + req_parser.get().method_string().to_string()
                         + "is not currently supported");
        return;
    }
    else
    {
        std::string target = req_parser.get().target().to_string();
        std::string handler_path = match(target.substr(0, target.find("/", 1)));
        if (handler_path == "") {
            Logger::logError("Session - no matching handler for " + target + " found.");
            return;
        }

        factory = routes_[handler_path];
        req_handler = factory->create(handler_path, target);
        Logger::logInfo("Session - Used factory to create request handler.");
    }
    req_handler->serve(data, bytes_transferred,res_);
    http::async_write(socket_,
            res_,
            boost::bind(&session::handle_write, this,
            boost::asio::placeholders::error));
    Logger::logInfo("Session - Request handler successfully wrote response.");
    delete req_handler;
  }
  else
  {
    Logger::logError("Session - Handle Read: Failed");
    delete this;
  }
}

// TODO: Add unit tests for this method
// Match the target path with the longest matching handler prefix.
// Returns empty string "" if no matches are found.
std::string session::match(std::string target)
{
    std::string result = "";
    int match_len = INT_MAX;
    for (const auto& route : routes_) 
    {
        std::string route_str = route.first;
        if (route_str.find(target) == 0 && route_str.size() < match_len)
        {
            result = route_str;
            match_len = route_str.size();
        }
    }
    Logger::logInfo("Target " + target + " has been matched with route " + result);
    return result;
}

void session::handle_write(const boost::system::error_code& error)
{
  if (!error)
  {
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&session::handle_read, this,
          boost::asio::placeholders::error,
          data_,
          boost::asio::placeholders::bytes_transferred));
    Logger::logInfo("Session - Handle Write: Success");
  } else {
    Logger::logInfo("Session - Handle Write: No more data to write. Closing session.");
    delete this;
  }
}
