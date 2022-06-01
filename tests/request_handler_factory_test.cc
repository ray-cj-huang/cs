#include <boost/system/error_code.hpp>
#include <string.h>
#include <unordered_map>
#include <boost/beast/http.hpp>

#include "gtest/gtest.h"
#include "request_handler.h"
#include "error_request_handler.h"
#include "static_request_handler.h"
#include "echo_request_handler.h"
#include "crud_request_handler.h"
#include "health_request_handler.h"
#include "caption_this_request_handler.h"
#include "request_handler_factory.h"
#include "error_request_handler_factory.h"
#include "static_request_handler_factory.h"
#include "echo_request_handler_factory.h"
#include "crud_request_handler_factory.h"
#include "sleep_request_handler_factory.h"
#include "health_request_handler_factory.h"
#include "caption_this_request_handler_factory.h"
#include "file_system_fake.h"

namespace beast = boost::beast;
namespace http = beast::http;

std::mutex mutex_ffs;

class RequestHandlerFactoryTest : public ::testing::Test {
  protected:
    
    const std::string TEST_ROOT = "../tests/static_files/";
    FileSystem* ffs = new FakeFileSystem(mutex_ffs);
    error_request_handler_factory err_rh_factory = error_request_handler_factory();
    echo_request_handler_factory erh_factory = echo_request_handler_factory();
    static_request_handler_factory srh_factory = static_request_handler_factory(TEST_ROOT);
    crud_request_handler_factory crh_factory = crud_request_handler_factory(TEST_ROOT, ffs);
    health_request_handler_factory hrh_factory = health_request_handler_factory();
    sleep_request_handler_factory slrh_factory = sleep_request_handler_factory();
    FileSystem* ffs2 = new FakeFileSystem(mutex_ffs);
    caption_this_request_handler_factory ctrh_factory = caption_this_request_handler_factory(TEST_ROOT, ffs2);

    void testErrorRHCreate(std::string location, std::string url) {
      request_handler* err_rh = err_rh_factory.create(location, url);
      EXPECT_EQ(err_rh->location_, location);
      EXPECT_EQ(err_rh->url_, url);
      delete err_rh;
    }
    
    void testERHCreate(std::string location, std::string url) {
      request_handler* erh = erh_factory.create(location, url);
      EXPECT_EQ(erh->location_, location);
      EXPECT_EQ(erh->url_, url);
      delete erh;
    }

    void testSRHCreate(std::string location, std::string url) {
      request_handler* srh = srh_factory.create(location, url);
      EXPECT_EQ(srh->location_, location);
      EXPECT_EQ(srh->url_, url);
      delete srh;
    }

    void testCRHCreate(std::string location, std::string url) {
      request_handler* crh = crh_factory.create(location, url);
      EXPECT_EQ(crh->location_, location);
      EXPECT_EQ(crh->url_, url);
      delete crh;
      delete ffs;
    }

    void testHRHCreate(std::string location, std::string url) {
      request_handler* hrh = hrh_factory.create(location, url);
      EXPECT_EQ(hrh->location_, location);
      EXPECT_EQ(hrh->url_, url);
      delete hrh;
    }

    void testSLRHCreate(std::string location, std::string url) {
      request_handler* slrh = slrh_factory.create(location, url);
      EXPECT_EQ(slrh->location_, location);
      EXPECT_EQ(slrh->url_, url);
      delete slrh;
    }

    void testCTRHCreate(std::string location, std::string url) {
      request_handler* ctrh = ctrh_factory.create(location, url);
      EXPECT_EQ(ctrh->location_, location);
      EXPECT_EQ(ctrh->url_, url);
      delete ctrh;
      delete ffs2;
    }
};

TEST_F(RequestHandlerFactoryTest, ErrorRHFactoryCreate) {
  testErrorRHCreate("/", "/");
}

TEST_F(RequestHandlerFactoryTest, ERHFactoryCreate) {
  testERHCreate("/echo", "/echo");
}

TEST_F(RequestHandlerFactoryTest, SRHFactoryCreate) {
  testSRHCreate("/static", "/static");
}

TEST_F(RequestHandlerFactoryTest, CRHFactoryCreate) {
  testCRHCreate("/crud", "/crud");
}

TEST_F(RequestHandlerFactoryTest, HRHFactoryCreate) {
  testHRHCreate("/echo", "/echo");
}

TEST_F(RequestHandlerFactoryTest, SLRHFactoryCreate) {
  testSLRHCreate("/sleep", "/sleep");
}

TEST_F(RequestHandlerFactoryTest, CTRHFactoryCreate) {
  testCTRHCreate("/caption", "/caption");
}