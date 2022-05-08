#include <boost/system/error_code.hpp>
#include <string.h>
#include <unordered_map>
#include <boost/beast/http.hpp>

#include "gtest/gtest.h"
#include "request_handler.h"
#include "static_request_handler.h"
#include "echo_request_handler.h"
#include "request_handler_factory.h"
#include "static_request_handler_factory.h"
#include "echo_request_handler_factory.h"

namespace beast = boost::beast;
namespace http = beast::http;

class RequestHandlerFactoryTest : public ::testing::Test {
  protected:
    
    const std::string TEST_ROOT = "../tests/static_files/";
    echo_request_handler_factory erh_factory = echo_request_handler_factory();
    static_request_handler_factory srh_factory = static_request_handler_factory(TEST_ROOT);

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
};

TEST_F(RequestHandlerFactoryTest, ERHFactoryCreate) {
  testERHCreate("/echo", "/echo");
}

TEST_F(RequestHandlerFactoryTest, SRHFactoryCreate) {
  testSRHCreate("/echo", "/echo");
}

