#include "gtest/gtest.h"
#include "config_parser.h"
#include "logger.h"

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

    void testGetPaths(std::unordered_map<std::string, std::string> &static_paths, 
                      std::unordered_set<std::string> &echo_paths,
                      bool correct) {
      out_config.GetPaths(out_config.static_paths_, out_config.echo_paths_);
      bool equal_static = out_config.static_paths_ == static_paths;
      bool equal_echo = out_config.echo_paths_ == echo_paths;
      
      if (correct) {
        EXPECT_TRUE(equal_static && equal_echo);
      }
      else {
        EXPECT_FALSE(equal_static && equal_echo);
      }
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

/* Right Test Cases for Endpoints Parsing */

TEST_F(ConfigParserTest, EndpointsCorrectConfig) {

  testParsing("endpoints_correct_config", true);
  std::unordered_map<std::string, std::string> expected_static( {{ "/static", "../static/" },
                                                                 { "/static1", "../tests/static_files/"}} );
  std::unordered_set<std::string> expected_echo( { "/echo", "/echo1" } );                                                          
  testGetPaths(expected_static, expected_echo, true);
}

TEST_F(ConfigParserTest, EndpointsRedefinedPathConfig) {
  testParsing("endpoints_redefined_path_config", true);
  std::unordered_map<std::string, std::string> expected_static( {{ "/static", "../static/" }});
  std::unordered_set<std::string> expected_echo( { "/echo", "/echo1" } );  
  testGetPaths(expected_static, expected_echo, true);
}

/*  Wrong Test Cases for Endpoints Parsing */

TEST_F(ConfigParserTest, EndpointsEchoFewArgumentsConfig) {
  testParsing("endpoints_echo_few_arguments_config", true);
  std::unordered_map<std::string, std::string> expected_static( {{ "/static", "../static/" },
                                                                 { "/static1", "../tests/static_files/"}} );
  std::unordered_set<std::string> expected_echo( { "/echo", "/echo1" } );  
  testGetPaths(expected_static, expected_echo, false);
}

TEST_F(ConfigParserTest, EndpointsStaticFewArgumentsConfig) {
  testParsing("endpoints_static_few_arguments_config", true);
  std::unordered_map<std::string, std::string> expected_static( {{ "/static", "../static/" },
                                                                 { "/static1", "../tests/static_files/"}} );
  std::unordered_set<std::string> expected_echo( { "/echo", "/echo1" } );  
  testGetPaths(expected_static, expected_echo, false);
}
