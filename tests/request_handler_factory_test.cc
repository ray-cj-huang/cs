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
#include "request_handler_factory.h"
#include "error_request_handler_factory.h"
#include "static_request_handler_factory.h"
#include "echo_request_handler_factory.h"
#include "crud_request_handler_factory.h"
#include "health_request_handler_factory.h"
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

    void testErrorRHCreate(std::string location, std::string url) {
      request_handler* err_rh = err_rh_factory.create(location, url);
      EXPECT_EQ(err_rh->location_, "/echo");
      EXPECT_EQ(err_rh->url_, "/echo");
      delete err_rh;
    }
    
    void testERHCreate(std::string location, std::string url) {
      request_handler* erh = erh_factory.create(location, url);
      EXPECT_EQ(erh->location_, "/echo");
      EXPECT_EQ(erh->url_, "/echo");
      delete erh;
    }

    void testSRHCreate(std::string location, std::string url) {
      request_handler* srh = srh_factory.create(location, url);
      EXPECT_EQ(srh->location_, "/echo");
      EXPECT_EQ(srh->url_, "/echo");
      delete srh;
    }

    void testCRHCreate(std::string location, std::string url) {
      request_handler* crh = crh_factory.create(location, url);
      EXPECT_EQ(crh->location_, "/echo");
      EXPECT_EQ(crh->url_, "/echo");
      delete crh;
      delete ffs;
    }

    void testHRHCreate(std::string location, std::string url) {
      request_handler* hrh = hrh_factory.create(location, url);
      EXPECT_EQ(hrh->location_, "/echo");
      EXPECT_EQ(hrh->url_, "/echo");
      delete hrh;
    }
};

TEST_F(RequestHandlerFactoryTest, ErrRHFactoryCreate) {
  testErrorRHCreate("/echo", "/echo");
}

TEST_F(RequestHandlerFactoryTest, ERHFactoryCreate) {
  testERHCreate("/echo", "/echo");
}

TEST_F(RequestHandlerFactoryTest, SRHFactoryCreate) {
  testSRHCreate("/echo", "/echo");
}

TEST_F(RequestHandlerFactoryTest, CRHFactoryCreate) {
  testCRHCreate("/echo", "/echo");
}

TEST_F(RequestHandlerFactoryTest, HRHFactoryCreate) {
  testHRHCreate("/echo", "/echo");
}
