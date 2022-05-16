#include <boost/system/error_code.hpp>
#include <string.h>
#include <boost/beast/http.hpp>

#include "gtest/gtest.h"
#include "session.h"
#include "request_handler_factory.h"
#include "error_request_handler_factory.h"
#include "echo_request_handler_factory.h"
#include "static_request_handler_factory.h"

namespace beast = boost::beast;
namespace http = beast::http;

std::unordered_map<std::string, request_handler_factory*> TEST_ROUTES = {
  {"/", new error_request_handler_factory()},
  {"/echo", new echo_request_handler_factory()},
  {"/echo2", new echo_request_handler_factory()},
  {"/static", new static_request_handler_factory("../tests/static_files/")}
};

class SessionTest : public ::testing::Test {
  protected:
    boost::asio::io_service io_service_;
    session session_ = session(io_service_, TEST_ROUTES);

    // NOTE: consider refactoring handle_write() to use dependency injection for socket
    // exposes handle_read() method and returns http response
    http::response<http::buffer_body> testHandleRead(const boost::system::error_code& error,
          char* data, size_t bytes_transferred) {
        // allocate on heap in case we delete the session (in case of error)
        session* new_session = new session(io_service_, TEST_ROUTES);
        new_session->handle_read(error, data, bytes_transferred);
        return new_session->res_;
    }

    void testHandleWrite() {
      // allocate on heap in case we delete the session (in case of error)
      session* new_session = new session(io_service_, TEST_ROUTES);
      new_session->handle_write();
    }

    std::string testMatch(std::string target) {
      return session_.match(target);
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

TEST_F(SessionTest, HandleReadBadHTTP) {
  char test_string[] = "BAD REQUEST\r\n\r\n";
  int test_num = 1026;
  boost::system::error_code no_error = boost::system::error_code();
  ASSERT_FALSE(no_error);

  http::response<http::buffer_body> res = testHandleRead(no_error, test_string, test_num);
  EXPECT_NE(res.body().data, test_string);
  EXPECT_NE(res.body().size, test_num);
}

TEST_F(SessionTest, HandleReadNonGet) {
  char test_string[] = "POST /echo HTTP/1.1\r\n\r\n";
  int test_num = 1026;
  boost::system::error_code no_error = boost::system::error_code();
  ASSERT_FALSE(no_error);

  http::response<http::buffer_body> res = testHandleRead(no_error, test_string, test_num);
  EXPECT_EQ(res.body().data, test_string);
  EXPECT_EQ(res.body().size, test_num);
}

TEST_F(SessionTest, HandleReadBadPath) {
  char test_string[] = "GET /notaprefix HTTP/1.1\r\n\r\n";
  int test_num = 1026;
  boost::system::error_code no_error = boost::system::error_code();
  ASSERT_FALSE(no_error);

  http::response<http::buffer_body> res = testHandleRead(no_error, test_string, test_num);
  EXPECT_NE(res.body().data, test_string);
  EXPECT_NE(res.body().size, test_num);
}

TEST_F(SessionTest, MatchEmpty) {
  EXPECT_EQ(testMatch(""), "/");
}

TEST_F(SessionTest, MatchExact) {
  EXPECT_EQ(testMatch("/echo"), "/echo");
}

TEST_F(SessionTest, MatchPrefix) {
  EXPECT_EQ(testMatch("/echo/location"), "/echo");
}

TEST_F(SessionTest, MatchPrefixLongest) {
  EXPECT_EQ(testMatch("/echo2location"), "/echo2");
}

TEST_F(SessionTest, MatchNoMatches) {
  EXPECT_EQ(testMatch("/foobar"), "/");
}

TEST_F(SessionTest, HandleWriteSucceeds) {
  testHandleWrite();
  SUCCEED();
}
