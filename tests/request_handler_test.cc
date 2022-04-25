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

    bool testEchoPut(char* buf, size_t size) {
        echo_request_handler erh = echo_request_handler();
        erh.put_data(buf, size);
        return erh.req_data_ == buf && erh.req_size_ == size;
    }

    bool testStaticPut(char* buf, size_t size) {
        std::unordered_map<std::string, std::string> static_paths(
            {{"/static", "../tests/static_files/"}});
        static_request_handler srh = static_request_handler(static_paths);
        srh.put_data(buf, size);
        return srh.req_data_ == buf && srh.req_size_ == size;
    }

    void testEchoHandler(char* buf, size_t size) {
        echo_request_handler erh = echo_request_handler();
        erh.put_data(buf, size);
        erh.write_response(res);
    }

    void testStaticHandler(char* buf, size_t size) {
        std::unordered_map<std::string, std::string> static_paths(
            {{"/static", "../tests/static_files/"}});
        static_request_handler srh = static_request_handler(static_paths);
        srh.put_data(buf, size);
        srh.write_response(res);
    }
};

TEST_F(RequestHandlerTest, putDataSucceeds) {
    char buf[] = "hi";
    size_t size = std::strlen(buf);
    EXPECT_EQ(testEchoPut(buf, size), true);
    EXPECT_EQ(testStaticPut(buf, size), true);
}

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

TEST_F(RequestHandlerTest, staticWriteUsupported) {
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
