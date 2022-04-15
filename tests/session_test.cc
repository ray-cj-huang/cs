#include <boost/system/error_code.hpp>

#include "gtest/gtest.h"
#include "session.h"

class SessionTest : public ::testing::Test {
  protected:
    boost::asio::io_service io_service_;
    session session_ = session(io_service_);

    // NOTE: consider refactoring handle_write() to use dependency injection for socket
    // exposes handle_read() method and returns http response
    http::response<http::buffer_body> testHandleRead(const boost::system::error_code& error,
          char* data, size_t bytes_transferred) {
        // allocate on heap in case we delete the session (in case of error)
        session* new_session = new session(io_service_);
        new_session->handle_read(error, data, bytes_transferred);
        return new_session->res_;
    }

    void testHandleWrite(const boost::system::error_code& error) {
      // allocate on heap in case we delete the session (in case of error)
      session* new_session = new session(io_service_);
      new_session->handle_write(error);
    }
};

TEST_F(SessionTest, SocketExists) {
  session_.socket();
  SUCCEED();
}

TEST_F(SessionTest, StartSucceeds) {
  session_.start();
  SUCCEED();
}

TEST_F(SessionTest, HandleReadSucceeds) {
  char test_string[] = "foo";
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