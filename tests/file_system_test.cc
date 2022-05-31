#include "gtest/gtest.h"
#include "file_system_fake.h"
#include "file_system_real.h"
#include <string>
#include <boost/filesystem.hpp>

std::mutex mutex_ffs;

namespace fs = boost::filesystem;

class FileSystemTest : public ::testing::Test {
  protected:
    FileSystemTest(): ffs_(mutex_ffs), rfs_(mutex_ffs) {}
    FakeFileSystem ffs_;
    RealFileSystem rfs_;
};

TEST_F(FileSystemTest, FakeAddFile) {
    EXPECT_TRUE(ffs_.upload_file("/1", "Hello 1"));
    EXPECT_TRUE(ffs_.exists("/1"));
    EXPECT_FALSE(ffs_.is_directory("/1"));
}

TEST_F(FileSystemTest, FakeAddDirectory) {
    EXPECT_TRUE(ffs_.create_directories("/usr"));
    EXPECT_TRUE(ffs_.exists("/usr"));
    EXPECT_TRUE(ffs_.is_directory("/usr"));
}

TEST_F(FileSystemTest, FakeAddDirectories) {
    EXPECT_TRUE(ffs_.create_directories("/usr/src"));
    EXPECT_TRUE(ffs_.exists("/usr"));
    EXPECT_TRUE(ffs_.is_directory("/usr"));
    EXPECT_TRUE(ffs_.exists("/usr/src"));
    EXPECT_TRUE(ffs_.is_directory("/usr/src"));
}

TEST_F(FileSystemTest, FakeAddNestedFile) {
    EXPECT_TRUE(ffs_.upload_file("/usr/src/1", "Hello 1"));
    EXPECT_TRUE(ffs_.exists("/usr"));
    EXPECT_TRUE(ffs_.is_directory("/usr"));
    EXPECT_TRUE(ffs_.exists("/usr/src"));
    EXPECT_TRUE(ffs_.is_directory("/usr/src"));
    EXPECT_TRUE(ffs_.exists("/usr/src/1"));
    EXPECT_FALSE(ffs_.is_directory("/usr/src/1"));
}

TEST_F(FileSystemTest, FakeDeleteFile) {
    EXPECT_TRUE(ffs_.upload_file("/usr/src/1", "Hello 1"));
    EXPECT_TRUE(ffs_.remove("/usr/src/1"));
    EXPECT_FALSE(ffs_.exists("/usr/src/1"));
    EXPECT_TRUE(ffs_.exists("/usr/src/"));
    EXPECT_TRUE(ffs_.is_empty("/usr/src/"));
}

TEST_F(FileSystemTest, FakeDeleteUnknownFile) {
    EXPECT_FALSE(ffs_.remove("/usr/src/1"));
}

TEST_F(FileSystemTest, FakeUpdateExistingFile) {
    EXPECT_TRUE(ffs_.upload_file("/usr/src/1", "Hello 1"));
    EXPECT_TRUE(ffs_.upload_file("/usr/src/1", "Hello one"));
    EXPECT_EQ(((FakeFile*)ffs_.get_entry("/usr/src/1"))->file_content_, "Hello one");
}

TEST_F(FileSystemTest, FakeListDirectory) {
    EXPECT_TRUE(ffs_.upload_file("/usr/src/1", "Hello 1"));
    EXPECT_TRUE(ffs_.upload_file("/usr/src/2", "Hello one"));
    std::string list_str;
    EXPECT_TRUE(ffs_.list_directory("/usr/src", list_str));
    EXPECT_EQ(list_str, "[1, 2]");
    EXPECT_EQ(ffs_.get_next_id("/usr/src"), 3);
}

TEST_F(FileSystemTest, RealAddFile) {
    EXPECT_TRUE(rfs_.upload_file("./FileTest", "Hello 1"));
    EXPECT_TRUE(rfs_.exists("./FileTest"));
    EXPECT_FALSE(rfs_.is_directory("./FileTest"));
    EXPECT_TRUE(rfs_.remove("./FileTest"));
}

TEST_F(FileSystemTest, RealAddDirectory) {
    EXPECT_TRUE(rfs_.create_directories("./FileTest"));
    EXPECT_TRUE(rfs_.exists("./FileTest"));
    EXPECT_TRUE(rfs_.is_directory("./FileTest"));
    EXPECT_TRUE(fs::remove_all("./FileTest"));
}

TEST_F(FileSystemTest, RealAddDirectories) {
    EXPECT_TRUE(rfs_.create_directories("./FileTest/src"));
    EXPECT_TRUE(rfs_.exists("./FileTest"));
    EXPECT_TRUE(rfs_.is_directory("./FileTest"));
    EXPECT_TRUE(rfs_.exists("./FileTest/src"));
    EXPECT_TRUE(rfs_.is_directory("./FileTest/src"));
    EXPECT_TRUE(fs::remove_all("./FileTest/src"));
}

TEST_F(FileSystemTest, RealDeleteFile) {
    EXPECT_TRUE(rfs_.create_directories("./FileTest/src"));
    EXPECT_TRUE(rfs_.upload_file("./FileTest/src/1", "Hello 1"));
    EXPECT_TRUE(rfs_.remove("./FileTest/src/1"));
    EXPECT_FALSE(rfs_.exists("./FileTest/src/1"));
    EXPECT_TRUE(rfs_.exists("./FileTest/src/"));
    EXPECT_TRUE(rfs_.is_empty("./FileTest/src/"));
    EXPECT_TRUE(fs::remove_all("./FileTest"));
}

TEST_F(FileSystemTest, RealDeleteUnknownFile) {
    EXPECT_FALSE(rfs_.remove("./FileTest/src/1"));
}

TEST_F(FileSystemTest, RealUpdateExistingFile) {
    EXPECT_TRUE(rfs_.upload_file("./FileTest", "Hello 1"));
    EXPECT_TRUE(rfs_.upload_file("./FileTest", "Hello one"));
    std::string data = "";
    EXPECT_TRUE(rfs_.read("./FileTest", data));
    EXPECT_EQ(data, "Hello one\n");
    EXPECT_TRUE(fs::remove_all("./FileTest"));
}

TEST_F(FileSystemTest, RealListDirectory) {
    EXPECT_TRUE(rfs_.create_directories("./FileTest/src"));
    EXPECT_TRUE(rfs_.upload_file("./FileTest/src/1", "Hello 1"));
    EXPECT_TRUE(rfs_.upload_file("./FileTest/src/2", "Hello one"));
    std::string list_str;
    EXPECT_TRUE(rfs_.list_directory("./FileTest/src", list_str));
    EXPECT_EQ(list_str, "[1, 2]");
    EXPECT_EQ(rfs_.get_next_id("./FileTest/src"), 3);
    EXPECT_TRUE(fs::remove_all("./FileTest"));
}

TEST_F(FileSystemTest, pathCat) {
    EXPECT_EQ(ffs_.path_cat("/super/", "/cool/"), "/super/cool/");
    EXPECT_EQ(ffs_.path_cat("/super/", "/"), "/super/");
    EXPECT_EQ(ffs_.path_cat("", "/super/cool"), "/super/cool");
    EXPECT_EQ(ffs_.path_cat("super", "cool"), "super/cool");
}
