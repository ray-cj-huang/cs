#include <boost/system/error_code.hpp>

#include "gtest/gtest.h"
#include "server.h"
#include "session.h"

static const int TEST_PORT = 80;
static std::unordered_map<std::string, std::string> TEST_MAP = std::unordered_map<std::string, std::string>();
static std::unordered_set<std::string> TEST_SET = std::unordered_set<std::string>();

class ServerTest : public ::testing::Test {
  protected:
    boost::asio::io_service io_service_;
    server server_ = server(io_service_, static_cast<short>(TEST_PORT), TEST_MAP, TEST_SET);
    
    // exposes handle_accept() method and returns HandleAcceptFlag to verify execution
    server::HandleAcceptFlag testHandleAccept(const boost::system::error_code& error) {
      // allocate on heap in case we delete the session (in case of error)
      session* new_session = new session(io_service_, TEST_MAP, TEST_SET);
      server_.handle_accept(new_session, error);
      return server_.flag;
    }
};

TEST_F(ServerTest, HandleAcceptSucceeds) {
  boost::system::error_code no_error = boost::system::error_code();
  ASSERT_FALSE(no_error);
  server::HandleAcceptFlag flag = testHandleAccept(no_error);
  EXPECT_EQ(flag, server::HandleAcceptFlag::OK);
}

TEST_F(ServerTest, HandleAcceptError) {
  boost::system::error_code error = boost::system::errc::make_error_code(boost::system::errc::not_supported);
  ASSERT_TRUE(error);
  server::HandleAcceptFlag flag = testHandleAccept(error);
  EXPECT_EQ(flag, server::HandleAcceptFlag::ERROR);
}
