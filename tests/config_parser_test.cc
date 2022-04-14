#include "gtest/gtest.h"
#include "config_parser.h"

static const std::string TEST_CONFIG_PATH = "test_configs/";

class ConfigParserTest : public ::testing::Test {
  protected:
    NginxConfigParser parser;
    NginxConfig out_config;
    
    void testParsing(const char* name, bool correct) {
      std::string filename = TEST_CONFIG_PATH + std::string(name);
      bool success = parser.Parse(filename.c_str(), &out_config);
      if (correct)
        EXPECT_TRUE(success);
      else
        EXPECT_FALSE(success);
    }
};

/* Right Test Cases */

TEST_F(ConfigParserTest, ExampleConfig) {
  testParsing("example_config", true);
}

TEST_F(ConfigParserTest, EmptyConfig) {
  testParsing("empty_config", true);
}

TEST_F(ConfigParserTest, ServerConfig) {
  testParsing("server_config", true);
}

TEST_F(ConfigParserTest, ProxyDefConfig) {
  testParsing("proxy_definition_config", true);
}

TEST_F(ConfigParserTest, ProxyConfig) {
  testParsing("proxy_config", true);
}

TEST_F(ConfigParserTest, MultiLocationsConfig) {
  testParsing("multi_locations_config", true);
}

TEST_F(ConfigParserTest, FastCGIConfig) {
  testParsing("fastCGI_config", true);
}

/* Wrong Test Cases */
TEST_F(ConfigParserTest, MalformedExtraBracketPriorConfig) {
  testParsing("malformed_extra_bracket_prior_config", false);
}

TEST_F(ConfigParserTest, MalformedRandomExtraBracketConfig) {
  testParsing("malformed_random_brackets_config", false);
}

TEST_F(ConfigParserTest, MalformedExtraSemicolonConfig) {
  testParsing("malformed_extra_semicolon_config", false);
}

TEST_F(ConfigParserTest, MalformedMultiContextsConfig) {
  testParsing("malformed_multi_contexts_config", false);
}
