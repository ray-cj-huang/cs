#include "gtest/gtest.h"
#include "caption_file_parser.h"

class CaptionFileParserTest : public ::testing::Test {
    protected:
        caption_file_parser cfp_;
};

TEST_F(CaptionFileParserTest, EmptyData) {
    caption_file res = cfp_.read("");
    EXPECT_EQ(res.top_caption, "");
    EXPECT_EQ(res.bot_caption, "");
    EXPECT_EQ(res.img_url, "");
}

TEST_F(CaptionFileParserTest, StandardData) {
    caption_file res = cfp_.read("foo\nbar\nhttps://foo.com/bar.jpg");
    EXPECT_EQ(res.top_caption, "foo");
    EXPECT_EQ(res.bot_caption, "bar");
    EXPECT_EQ(res.img_url, "https://foo.com/bar.jpg");
}

TEST_F(CaptionFileParserTest, NoBottomText) {
    caption_file res = cfp_.read("foo\n\nhttps://foo.com/bar.jpg");
    EXPECT_EQ(res.top_caption, "foo");
    EXPECT_EQ(res.bot_caption, "");
    EXPECT_EQ(res.img_url, "https://foo.com/bar.jpg");
}

TEST_F(CaptionFileParserTest, NoText) {
    caption_file res = cfp_.read("\n\nhttps://foo.com/bar.jpg");
    EXPECT_EQ(res.top_caption, "");
    EXPECT_EQ(res.bot_caption, "");
    EXPECT_EQ(res.img_url, "https://foo.com/bar.jpg");
}

TEST_F(CaptionFileParserTest, NoImage) {
    caption_file res = cfp_.read("foo\nbar");
    EXPECT_EQ(res.top_caption, "foo");
    EXPECT_EQ(res.bot_caption, "bar");
    EXPECT_EQ(res.img_url, "");
}

TEST_F(CaptionFileParserTest, ExtraData) {
    caption_file res = cfp_.read("foo\nbar\nhttps://foo.com/bar.jpg\nzamn");
    EXPECT_EQ(res.top_caption, "foo");
    EXPECT_EQ(res.bot_caption, "bar");
    EXPECT_EQ(res.img_url, "https://foo.com/bar.jpg");
}