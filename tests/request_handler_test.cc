#include <boost/system/error_code.hpp>
#include <algorithm>
#include <string.h>
#include <unordered_map>
#include <boost/beast/http.hpp>

#include "gtest/gtest.h"
#include "request_handler.h"
#include "crud_request_handler.h"
#include "error_request_handler.h"
#include "static_request_handler.h"
#include "echo_request_handler.h"
#include "health_request_handler.h"
#include "sleep_request_handler.h"
#include "caption_this_request_handler.h"
#include "caption_file_parser.h"

#include "file_system_fake.h"

namespace beast = boost::beast;
namespace http = beast::http;

std::mutex mutex_ffs;

class RequestHandlerTest : public ::testing::Test {
  protected:
    void SetUp() override {
        fs = new FakeFileSystem(mutex_ffs, "/crud_root");
        cfp = new caption_file_parser();
    }
    http::response<http::buffer_body> res;
    FileSystem* fs;
    caption_file_parser* cfp;


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

    void testHealthHandler(char* buf, size_t size) {
        health_request_handler hrh = health_request_handler("/health", "/health");
        hrh.serve(buf, size, res);
    }

    void testSleepHandler(char* buf, size_t size) {
        sleep_request_handler slrh = sleep_request_handler("/sleep", "/sleep");
        slrh.serve(buf, size, res);
    }

    void testCaptionThisHandler(char* buf, size_t size) {
        caption_this_request_handler ctrh = caption_this_request_handler("/caption", 
            "/whatever/since/this/is/never/used", "/caption_root", fs, cfp);
        ctrh.serve(buf, size, res);
    }
};

TEST_F(RequestHandlerTest, error400ServeSucceeds) {
    char buf[] = "hi";
    size_t size = std::strlen(buf);
    testErrorHandler(buf, size);
    EXPECT_EQ(res.result(), http::status::bad_request);
    EXPECT_EQ(res.base()[http::field::content_type], "text/html");
    EXPECT_NE(res.body().data, buf);
    EXPECT_NE(res.body().size, size);
}

TEST_F(RequestHandlerTest, error404ServeSucceeds) {
    char buf[] = "GET /static/doesnotexist.dne HTTP/1.1\r\n\r\n";
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

TEST_F(RequestHandlerTest, sleepServeSucceeds) {
    char buf[] = "foobar";
    size_t size = std::strlen(buf);
    testSleepHandler(buf, size);
    EXPECT_EQ(res.result(), http::status::ok);
    EXPECT_EQ(res.base()[http::field::content_type], "text/plain");

    std::string res_body;
    for (int i = 0; i < res.body().size; i++) {
        res_body += ((char*)res.body().data)[i];
    }
    std::string res_body_expected = "Thread slept for 5 seconds.";
    EXPECT_EQ(res_body, res_body_expected);
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

TEST_F(RequestHandlerTest, crudServeDelete) {
    char buf_post[] = "POST /crud/Shoe HTTP/1.1\r\n\r\n";
    size_t size = std::strlen(buf_post);
    testCRUDHandler(buf_post, size);
    EXPECT_EQ(res.result(), http::status::ok);

    char buf_del[] = "DELETE /crud/Shoe/1 HTTP/1.1\r\n\r\n";
    size = std::strlen(buf_del);
    testCRUDHandler(buf_del, size);
    EXPECT_EQ(res.result(), http::status::ok);

    std::string res_body;
    for (int i = 0; i < res.body().size; i++) {
        res_body += ((char*)res.body().data)[i];
    }
    std::string res_body_expected = "File: /crud_root/Shoe/1 deleted";
    EXPECT_EQ(res_body, res_body_expected);
}

TEST_F(RequestHandlerTest, crudServeDeleteUnknownID) {
    char buf_post[] = "POST /crud/Shoe HTTP/1.1\r\n\r\n";
    size_t size = std::strlen(buf_post);
    testCRUDHandler(buf_post, size);
    EXPECT_EQ(res.result(), http::status::ok);

    char buf_del[] = "DELETE /crud/Shoe/2 HTTP/1.1\r\n\r\n";
    size = std::strlen(buf_del);
    testCRUDHandler(buf_del, size);
    EXPECT_EQ(res.result(), http::status::bad_request);
}

TEST_F(RequestHandlerTest, crudServeDeleteUnknownEntity) {
    char buf_post[] = "POST /crud/Shoe HTTP/1.1\r\n\r\n";
    size_t size = std::strlen(buf_post);
    testCRUDHandler(buf_post, size);
    EXPECT_EQ(res.result(), http::status::ok);

    char buf_del[] = "DELETE /crud/Clothes/1 HTTP/1.1\r\n\r\n";
    size = std::strlen(buf_del);
    testCRUDHandler(buf_del, size);
    EXPECT_EQ(res.result(), http::status::bad_request);
}

TEST_F(RequestHandlerTest, crudServeRetrieve) {
    char buf_post_1[] = "POST /crud/Shoe HTTP/1.1\r\nContent-Length: 9\r\n\r\nHello one";
    size_t size = std::strlen(buf_post_1);
    testCRUDHandler(buf_post_1, size);
    EXPECT_EQ(res.result(), http::status::ok);

    char buf_post_2[] = "POST /crud/Shoe HTTP/1.1\r\nContent-Length: 9\r\n\r\nHello two";
    size = std::strlen(buf_post_2);
    testCRUDHandler(buf_post_2, size);
    EXPECT_EQ(res.result(), http::status::ok);

    char buf_retrieve[] = "GET /crud/Shoe/1 HTTP/1.1\r\n\r\n";
    size = std::strlen(buf_retrieve);
    testCRUDHandler(buf_retrieve, size);
    EXPECT_EQ(res.result(), http::status::ok);

    std::string res_body;
    for (int i = 0; i < res.body().size; i++) {
        res_body += ((char*)res.body().data)[i];
    }
    std::string res_body_expected = "Hello one";
    EXPECT_EQ(res_body, res_body_expected);
}

TEST_F(RequestHandlerTest, crudServeUpdate) {
    char buf_post[] = "POST /crud/Shoe HTTP/1.1\r\nContent-Length: 11\r\n\r\n{ hi: foo }";
    size_t size = std::strlen(buf_post);
    testCRUDHandler(buf_post, size);
    EXPECT_EQ(res.result(), http::status::ok);

    char buf_put[] = "PUT /crud/Shoe/1 HTTP/1.1\r\nContent-Length: 12\r\n\r\n{ lo: barr }";
    size = std::strlen(buf_put);
    testCRUDHandler(buf_put, size);
    EXPECT_EQ(res.result(), http::status::ok);

    std::string res_body;
    for (int i = 0; i < res.body().size; i++) {
        res_body += ((char*)res.body().data)[i];
    }
    std::string res_body_expected = "File: /crud_root/Shoe/1 updated";
    EXPECT_EQ(res_body, res_body_expected);

    EXPECT_EQ(((FakeFile*)((FakeFileSystem*)fs)->get_entry("/crud_root/Shoe/1"))->file_content_, "{ lo: barr }");
}

TEST_F(RequestHandlerTest, crudServeList) {
    char buf_post_1[] = "POST /crud/Shoe HTTP/1.1\r\nContent-Length: 9\r\n\r\nHello one";
    size_t size = std::strlen(buf_post_1);
    testCRUDHandler(buf_post_1, size);
    EXPECT_EQ(res.result(), http::status::ok);

    char buf_post_2[] = "POST /crud/Shoe HTTP/1.1\r\nContent-Length: 9\r\n\r\nHello two";
    size = std::strlen(buf_post_2);
    testCRUDHandler(buf_post_2, size);
    EXPECT_EQ(res.result(), http::status::ok);

    char buf_list[] = "GET /crud/Shoe HTTP/1.1\r\n\r\n";
    size = std::strlen(buf_list);
    testCRUDHandler(buf_list, size);
    EXPECT_EQ(res.result(), http::status::ok);

    std::string res_body;
    for (int i = 0; i < res.body().size; i++) {
        res_body += ((char*)res.body().data)[i];
    }
    std::string res_body_expected = "[1, 2]";
    EXPECT_EQ(res_body, res_body_expected);
}

TEST_F(RequestHandlerTest, healthServe) {
    char buf[] = "GET /health HTTP/1.1\r\n\r\n";
    size_t size = std::strlen(buf);
    testHealthHandler(buf, size);
    EXPECT_EQ(res.result(), http::status::ok);
    EXPECT_EQ(res.base()[http::field::content_type], "text/plain");
    std::string res_body;
    for (int i = 0; i < res.body().size; i++) {
        res_body += ((char*)res.body().data)[i];
    }
    std::string res_body_expected = "OK";
    EXPECT_EQ(res_body, res_body_expected);
}

/**** Caption This Handler Unit Tests ****/

TEST_F(RequestHandlerTest, captionThisPostSubmission) {
    char buf[] = "POST /caption/submit HTTP/1.1\r\n\r\n";
    size_t size = std::strlen(buf);
    testCaptionThisHandler(buf, size);
    EXPECT_EQ(res.result(), http::status::ok);

    std::string res_body;
    for (int i = 0; i < res.body().size; i++) {
        res_body += ((char*)res.body().data)[i];
    }
    std::string res_body_expected = "{ \"id\": 1 }";
    EXPECT_EQ(res_body, res_body_expected);
}

TEST_F(RequestHandlerTest, captionThisGetSubmissionWithID) {
    char buf_post_1[] = "POST /caption/submit HTTP/1.1\r\nContent-Length: 17\r\n\r\ntop\nbottom\nimgurl";
    size_t size = std::strlen(buf_post_1);
    testCaptionThisHandler(buf_post_1, size);
    EXPECT_EQ(res.result(), http::status::ok);

    std::string specific_submit_page_path = "../static/specific_submission_page.html";
    std::string body = "[img] [top text] [bottom text]";

    fs->upload_file(specific_submit_page_path, body);

    char buf_retrieve[] = "GET /caption/1 HTTP/1.1\r\n\r\n";
    size = std::strlen(buf_retrieve);
    testCaptionThisHandler(buf_retrieve, size);
    EXPECT_EQ(res.result(), http::status::ok);

    std::string res_body;
    for (int i = 0; i < res.body().size; i++) {
        res_body += ((char*)res.body().data)[i];
    }
    std::string res_body_expected = "imgurl top bottom";
    EXPECT_EQ(res_body, res_body_expected);
}

TEST_F(RequestHandlerTest, captionThisGetSubmissionPage) {
    std::string submit_page_path = "../static/submit_page.html";
    std::string body = "submit [img] [img] page";

    fs->upload_file(submit_page_path, body);

    char buf_retrieve[] = "GET /caption/submit HTTP/1.1\r\n\r\n";
    size_t size = std::strlen(buf_retrieve);
    testCaptionThisHandler(buf_retrieve, size);
    EXPECT_EQ(res.result(), http::status::ok);
}

TEST_F(RequestHandlerTest, captionThisGalleryPage) {
    char buf_post_1[] = "POST /caption/submit HTTP/1.1\r\nContent-Length: 17\r\n\r\ntop\nbottom\nimgurl";
    size_t size = std::strlen(buf_post_1);
    testCaptionThisHandler(buf_post_1, size);
    EXPECT_EQ(res.result(), http::status::ok);

    char buf_post_2[] = "POST /caption/submit HTTP/1.1\r\nContent-Length: 20\r\n\r\ntop_\nbottom_\nimgurl_";
    size = std::strlen(buf_post_2);
    testCaptionThisHandler(buf_post_2, size);
    EXPECT_EQ(res.result(), http::status::ok);

    std::string specific_gallery_page_path = "../static/gallery_page.html";
    std::string body = "PLACEHOLDER"; // replcae this with PLACEHOLDER

    fs->upload_file(specific_gallery_page_path, body);

    char buf_gallery[] = "GET /caption HTTP/1.1\r\n\r\n";
    size = std::strlen(buf_gallery);
    testCaptionThisHandler(buf_gallery, size);
    EXPECT_EQ(res.result(), http::status::ok);

    std::string res_gallery_body;
    for (int i = 0; i < res.body().size; i++) {
        res_gallery_body += ((char*)res.body().data)[i];
    }

    std::string res_gallery_body_expected =
        "<button class=\"relative text-white text-sm flex flex-wrap md:w-1/3 w-full hover:opacity-70\"> \
            <a href=\"/caption/" + std::string("1") + "\" class=\"w-full h-full p-1 md:p-2\"> \
                <img src=\"" + std::string("imgurl")  + "\"alt=\"Image\" class=\"block object-cover object-center w-full h-full rounded-lg\"> \
                <div class=\"absolute left-1/2 transform -translate-x-1/2 top-3 items-center break-words max-w-[80%]\">" + std::string("top") + "</div> \
                <div class=\"absolute left-1/2 transform -translate-x-1/2 bottom-3 items-center break-words max-w-[80%]\">" + std::string("bottom") + "</div> \
            </a> \
        </button><button class=\"relative text-white text-sm flex flex-wrap md:w-1/3 w-full hover:opacity-70\"> \
            <a href=\"/caption/" + std::string("2") + "\" class=\"w-full h-full p-1 md:p-2\"> \
                <img src=\"" + std::string("imgurl_")  + "\"alt=\"Image\" class=\"block object-cover object-center w-full h-full rounded-lg\"> \
                <div class=\"absolute left-1/2 transform -translate-x-1/2 top-3 items-center break-words max-w-[80%]\">" + std::string("top_") + "</div> \
                <div class=\"absolute left-1/2 transform -translate-x-1/2 bottom-3 items-center break-words max-w-[80%]\">" + std::string("bottom_") + "</div> \
            </a> \
        </button>";

    res_gallery_body.erase(remove(res_gallery_body.begin(), res_gallery_body.end(), ' '), res_gallery_body.end());
    res_gallery_body_expected.erase(remove(res_gallery_body_expected.begin(), res_gallery_body_expected.end(), ' '), res_gallery_body_expected.end());

    EXPECT_EQ(res_gallery_body, res_gallery_body_expected);
}

