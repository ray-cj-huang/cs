#include "gtest/gtest.h"
#include "file_system_fake.h"

std::mutex mutex_ffs;

namespace fs = boost::filesystem;

class FakeFileSystemTest : public ::testing::Test {
  protected:
    FakeFileSystemTest(): ffs_(mutex_ffs) {}
    FakeFileSystem ffs_;
};

TEST_F(FakeFileSystemTest, AddFile) {
    EXPECT_TRUE(ffs_.upload_file("/1", "Hello 1"));
    EXPECT_TRUE(ffs_.exists("/1"));
    EXPECT_FALSE(ffs_.is_directory("/1"));
}

TEST_F(FakeFileSystemTest, AddDirectory) {
    EXPECT_TRUE(ffs_.create_directories("/usr"));
    EXPECT_TRUE(ffs_.exists("/usr"));
    EXPECT_TRUE(ffs_.is_directory("/usr"));
}

TEST_F(FakeFileSystemTest, AddDirectories) {
    EXPECT_TRUE(ffs_.create_directories("/usr/src"));
    EXPECT_TRUE(ffs_.exists("/usr"));
    EXPECT_TRUE(ffs_.is_directory("/usr"));
    EXPECT_TRUE(ffs_.exists("/usr/src"));
    EXPECT_TRUE(ffs_.is_directory("/usr/src"));
}

TEST_F(FakeFileSystemTest, AddNestedFile) {
    EXPECT_TRUE(ffs_.upload_file("/usr/src/1", "Hello 1"));
    EXPECT_TRUE(ffs_.exists("/usr"));
    EXPECT_TRUE(ffs_.is_directory("/usr"));
    EXPECT_TRUE(ffs_.exists("/usr/src"));
    EXPECT_TRUE(ffs_.is_directory("/usr/src"));
    EXPECT_TRUE(ffs_.exists("/usr/src/1"));
    EXPECT_FALSE(ffs_.is_directory("/usr/src/1"));
}

TEST_F(FakeFileSystemTest, DeleteFile) {
    EXPECT_TRUE(ffs_.upload_file("/usr/src/1", "Hello 1"));
    EXPECT_TRUE(ffs_.remove("/usr/src/1"));
    EXPECT_FALSE(ffs_.exists("/usr/src/1"));
    EXPECT_TRUE(ffs_.exists("/usr/src/"));
    EXPECT_TRUE(ffs_.is_empty("/usr/src/"));
}

TEST_F(FakeFileSystemTest, DeleteUnknownFile) {
    EXPECT_FALSE(ffs_.remove("/usr/src/1"));
}

TEST_F(FakeFileSystemTest, UpdateExistingFile) {
    EXPECT_TRUE(ffs_.upload_file("/usr/src/1", "Hello 1"));
    EXPECT_TRUE(ffs_.upload_file("/usr/src/1", "Hello one"));
    EXPECT_EQ(((FakeFile*)ffs_.get_entry("/usr/src/1").get())->file_content_, "Hello one");
}