#include <boost/system/error_code.hpp>
#include <string.h>
#include <unordered_map>
#include <boost/beast/http.hpp>

#include "gtest/gtest.h"
#include "request_handler.h"
#include "static_request_handler.h"
#include "echo_request_handler.h"

namespace beast = boost::beast;
namespace http = beast::http;

class RequestHandlerTest : public ::testing::Test {
  protected:
    
    http::response<http::buffer_body> res;

    void testEchoHandler(char* buf, size_t size) {
        echo_request_handler erh = echo_request_handler("/echo", "/echo");
        erh.serve(buf, size, res);
    }

    void testStaticHandler(char* buf, size_t size) {
        static_request_handler srh = static_request_handler("/static", 
            "/static/foo", "../tests/static_files");
        srh.serve(buf, size, res);
    }
};

TEST_F(RequestHandlerTest, echoWriteSucceeds) {
    char buf[] = "hi";
    size_t size = std::strlen(buf);
    testEchoHandler(buf, size);
    EXPECT_EQ(res.result(), http::status::ok);
    EXPECT_EQ(res.base()[http::field::content_type], "text/plain");
    EXPECT_EQ(res.body().data, buf);
    EXPECT_EQ(res.body().size, size);
}

TEST_F(RequestHandlerTest, staticWriteTxt) {
    char buf[] = "GET /static/test.txt HTTP/1.1\r\n\r\n";
    size_t size = std::strlen(buf);
    testStaticHandler(buf, size);
    EXPECT_EQ(res.result(), http::status::ok);
    EXPECT_EQ(res.base()[http::field::content_type], "text/plain");
}

TEST_F(RequestHandlerTest, staticWriteHTML) {
    char buf[] = "GET /static/test.html HTTP/1.1\r\n\r\n";
    size_t size = std::strlen(buf);
    testStaticHandler(buf, size);
    EXPECT_EQ(res.result(), http::status::ok);
    EXPECT_EQ(res.base()[http::field::content_type], "text/html");
}

TEST_F(RequestHandlerTest, staticWriteJpg) {
    char buf[] = "GET /static/test.jpg HTTP/1.1\r\n\r\n";
    size_t size = std::strlen(buf);
    testStaticHandler(buf, size);
    EXPECT_EQ(res.result(), http::status::ok);
    EXPECT_EQ(res.base()[http::field::content_type], "image/jpeg");
}

TEST_F(RequestHandlerTest, staticWriteJpeg) {
    char buf[] = "GET /static/test.jpeg HTTP/1.1\r\n\r\n";
    size_t size = std::strlen(buf);
    testStaticHandler(buf, size);
    EXPECT_EQ(res.result(), http::status::ok);
    EXPECT_EQ(res.base()[http::field::content_type], "image/jpeg");
}

TEST_F(RequestHandlerTest, staticWriteZip) {
    char buf[] = "GET /static/test.zip HTTP/1.1\r\n\r\n";
    size_t size = std::strlen(buf);
    testStaticHandler(buf, size);
    EXPECT_EQ(res.result(), http::status::ok);
    EXPECT_EQ(res.base()[http::field::content_type], "application/zip");
}

TEST_F(RequestHandlerTest, staticWriteUnsupported) {
    char buf[] = "GET /static/test.unknown HTTP/1.1\r\n\r\n";
    size_t size = std::strlen(buf);
    testStaticHandler(buf, size);
    EXPECT_EQ(res.result(), http::status::ok);
    EXPECT_EQ(res.base()[http::field::content_type], "text/plain");
}

TEST_F(RequestHandlerTest, staticWriteFileNotExist) {
    char buf[] = "GET /static/doesntexist HTTP/1.1\r\n\r\n";
    size_t size = std::strlen(buf);
    testStaticHandler(buf, size);
    EXPECT_EQ(res.result(), http::status::not_found);
    EXPECT_EQ(res.base()[http::field::content_type], "text/html");
}
