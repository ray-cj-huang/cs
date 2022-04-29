#include <boost/system/error_code.hpp>
#include <string.h>
#include <boost/beast/http.hpp>

#include "gtest/gtest.h"
#include "session.h"

namespace beast = boost::beast;
namespace http = beast::http;

static std::unordered_set<std::string> TEST_SET = std::unordered_set<std::string>( { "/echo" });
static std::unordered_map<std::string, std::string> TEST_MAP = std::unordered_map<std::string, std::string>( { { "/static", "../tests/static_files/ "}});

class SessionTest : public ::testing::Test {
  protected:
    boost::asio::io_service io_service_;
    session session_ = session(io_service_,
                               new echo_request_handler(TEST_SET),
                               new static_request_handler(TEST_MAP));

    // NOTE: consider refactoring handle_write() to use dependency injection for socket
    // exposes handle_read() method and returns http response
    http::response<http::buffer_body> testHandleRead(const boost::system::error_code& error,
          char* data, size_t bytes_transferred) {
        // allocate on heap in case we delete the session (in case of error)
        session* new_session = new session(io_service_, 
                                           new echo_request_handler(TEST_SET),
                                           new static_request_handler(TEST_MAP));
        new_session->handle_read(error, data, bytes_transferred);
        return new_session->res_;
    }

    void testHandleWrite(const boost::system::error_code& error) {
      // allocate on heap in case we delete the session (in case of error)
      session* new_session = new session(io_service_,
                                         new echo_request_handler(TEST_SET),
                                         new static_request_handler(TEST_MAP));
      new_session->handle_write(error);
    }

    session::ParseRequestType testParseRequest(char* data)
    {
        return session_.parse_request(data);
    }
};

TEST_F(SessionTest, RequestParserTest) {
    http::request_parser<http::string_body> req_parser;
    std::string s = "GET /echo HTTP/1.1\r\n\r\n";
    boost::beast::error_code ec;
    size_t n_bytes = req_parser.put(boost::asio::buffer(s), ec);
    std::string is_done = req_parser.is_done() ? "true" : "false";
    std::string got_some = req_parser.got_some() ? "true" : "false";
    std::string is_header_done = req_parser.is_header_done() ? "true" : "false";
    Logger::logInfo(is_done);
    Logger::logInfo(got_some);
    Logger::logInfo(is_header_done);
    EXPECT_EQ(req_parser.get().method_string().to_string(), std::string("GET"));
}

TEST_F(SessionTest, SocketExists) {
  session_.socket();
  SUCCEED();
}

TEST_F(SessionTest, StartSucceeds) {
  session_.start();
  SUCCEED();
}

TEST_F(SessionTest, ParseRequestSucceeds) {
    char test_string[] = "GET /static HTTP/1.1\r\n\r\n";
    EXPECT_EQ(testParseRequest(test_string),
        session::ParseRequestType::STATICTYPE);
}

TEST_F(SessionTest, HandleReadSucceeds) {
  char test_string[] = "GET /echo HTTP/1.1\r\n\r\n";
  int test_num = 1025;
  boost::system::error_code no_error = boost::system::error_code();
  ASSERT_FALSE(no_error);

  http::response<http::buffer_body> res = testHandleRead(no_error, test_string, test_num);
  EXPECT_EQ(res.result(), http::status::ok);
  EXPECT_EQ(res.body().data, test_string);
  EXPECT_EQ(res.body().size, test_num);
}

TEST_F(SessionTest, HandleReadError) {
  char test_string[] = "bar";
  int test_num = 1026;
  boost::system::error_code error = boost::system::errc::make_error_code(boost::system::errc::not_supported);
  ASSERT_TRUE(error);

  http::response<http::buffer_body> res = testHandleRead(error, test_string, test_num);
  EXPECT_NE(res.body().data, test_string);
  EXPECT_NE(res.body().size, test_num);
}

TEST_F(SessionTest, HandleWriteSucceeds) {
  boost::system::error_code no_error = boost::system::error_code();
  ASSERT_FALSE(no_error);
  testHandleWrite(no_error);
  SUCCEED();
}

TEST_F(SessionTest, HandleWriteError) {
  boost::system::error_code error = boost::system::errc::make_error_code(boost::system::errc::not_supported);
  ASSERT_TRUE(error);
  testHandleWrite(error);
  SUCCEED();
}