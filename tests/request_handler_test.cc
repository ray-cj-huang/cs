#include <boost/system/error_code.hpp>
#include <string.h>
#include <unordered_map>
#include <boost/beast/http.hpp>

#include "gtest/gtest.h"
#include "request_handler.h"
#include "crud_request_handler.h"
#include "error_request_handler.h"
#include "static_request_handler.h"
#include "echo_request_handler.h"

#include "file_system_fake.h"

namespace beast = boost::beast;
namespace http = beast::http;

std::mutex mutex_ffs;

class RequestHandlerTest : public ::testing::Test {
  protected:
    void SetUp() override {
        fs = new FakeFileSystem(mutex_ffs, "/crud_root");
    }
    http::response<http::buffer_body> res;
    FileSystem* fs;

    void testErrorHandler(char* buf, size_t size) {
        error_request_handler err_rh = error_request_handler("/echo", "/echo");
        err_rh.serve(buf, size, res);
    }

    void testEchoHandler(char* buf, size_t size) {
        echo_request_handler erh = echo_request_handler("/echo", "/echo");
        erh.serve(buf, size, res);
    }

    void testStaticHandler(char* buf, size_t size) {
        static_request_handler srh = static_request_handler("/static", 
            "/static/foo", "../tests/static_files");
        srh.serve(buf, size, res);
    }

    void testCRUDHandler(char* buf, size_t size) {
        crud_request_handler crh = crud_request_handler("/crud", 
            "/whatever/since/this/is/never/used", "/crud_root", fs);
        crh.serve(buf, size, res);
    }
};

TEST_F(RequestHandlerTest, errorServeSucceeds) {
    char buf[] = "hi";
    size_t size = std::strlen(buf);
    testErrorHandler(buf, size);
    EXPECT_EQ(res.result(), http::status::not_found);
    EXPECT_EQ(res.base()[http::field::content_type], "text/html");
    EXPECT_NE(res.body().data, buf);
    EXPECT_NE(res.body().size, size);
}

TEST_F(RequestHandlerTest, echoServeSucceeds) {
    char buf[] = "hi";
    size_t size = std::strlen(buf);
    testEchoHandler(buf, size);
    EXPECT_EQ(res.result(), http::status::ok);
    EXPECT_EQ(res.base()[http::field::content_type], "text/plain");
    EXPECT_EQ(res.body().data, buf);
    EXPECT_EQ(res.body().size, size);
}

TEST_F(RequestHandlerTest, staticServeTxt) {
    char buf[] = "GET /static/test.txt HTTP/1.1\r\n\r\n";
    size_t size = std::strlen(buf);
    testStaticHandler(buf, size);
    EXPECT_EQ(res.result(), http::status::ok);
    EXPECT_EQ(res.base()[http::field::content_type], "text/plain");
}

TEST_F(RequestHandlerTest, staticServeHTML) {
    char buf[] = "GET /static/test.html HTTP/1.1\r\n\r\n";
    size_t size = std::strlen(buf);
    testStaticHandler(buf, size);
    EXPECT_EQ(res.result(), http::status::ok);
    EXPECT_EQ(res.base()[http::field::content_type], "text/html");
}

TEST_F(RequestHandlerTest, staticServeJpg) {
    char buf[] = "GET /static/test.jpg HTTP/1.1\r\n\r\n";
    size_t size = std::strlen(buf);
    testStaticHandler(buf, size);
    EXPECT_EQ(res.result(), http::status::ok);
    EXPECT_EQ(res.base()[http::field::content_type], "image/jpeg");
}

TEST_F(RequestHandlerTest, staticServeJpeg) {
    char buf[] = "GET /static/test.jpeg HTTP/1.1\r\n\r\n";
    size_t size = std::strlen(buf);
    testStaticHandler(buf, size);
    EXPECT_EQ(res.result(), http::status::ok);
    EXPECT_EQ(res.base()[http::field::content_type], "image/jpeg");
}

TEST_F(RequestHandlerTest, staticServeZip) {
    char buf[] = "GET /static/test.zip HTTP/1.1\r\n\r\n";
    size_t size = std::strlen(buf);
    testStaticHandler(buf, size);
    EXPECT_EQ(res.result(), http::status::ok);
    EXPECT_EQ(res.base()[http::field::content_type], "application/zip");
}

TEST_F(RequestHandlerTest, staticServeUnsupported) {
    char buf[] = "GET /static/test.unknown HTTP/1.1\r\n\r\n";
    size_t size = std::strlen(buf);
    testStaticHandler(buf, size);
    EXPECT_EQ(res.result(), http::status::ok);
    EXPECT_EQ(res.base()[http::field::content_type], "text/plain");
}

TEST_F(RequestHandlerTest, staticServeFileNotExist) {
    char buf[] = "GET /static/doesntexist HTTP/1.1\r\n\r\n";
    size_t size = std::strlen(buf);
    testStaticHandler(buf, size);
    EXPECT_EQ(res.result(), http::status::not_found);
    EXPECT_EQ(res.base()[http::field::content_type], "text/html");
}

/**** CRUD Handler Unit Tests ****/

TEST_F(RequestHandlerTest, crudServeCreate) {
    char buf[] = "POST /crud/Shoe HTTP/1.1\r\n\r\n";
    size_t size = std::strlen(buf);
    testCRUDHandler(buf, size);
    EXPECT_EQ(res.result(), http::status::ok);

    std::string res_body;
    for (int i = 0; i < res.body().size; i++) {
        res_body += ((char*)res.body().data)[i];
    }
    std::string res_body_expected = "{ \"id\": 1 }";
    EXPECT_EQ(res_body, res_body_expected);
}