#include "gtest/gtest.h"
#include "config_parser.h"
#include "logger.h"
#include <fstream>

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
                      std::unordered_map<std::string, std::string> &CRUD_paths,
                      std::unordered_set<std::string> &health_paths,
                      bool correct) {
      out_config.GetPaths(
          out_config.static_paths_,
          out_config.echo_paths_,
          out_config.CRUD_paths_,
          out_config.health_paths_);
      bool equal_static = out_config.static_paths_ == static_paths;
      bool equal_echo = out_config.echo_paths_ == echo_paths;
      bool equal_CRUD = out_config.CRUD_paths_ == CRUD_paths;
      bool equal_health = out_config.health_paths_ == health_paths;
      
      if (correct) {
        EXPECT_TRUE(equal_static && equal_echo && equal_CRUD && equal_health);
      }
      else {
        EXPECT_FALSE(equal_static && equal_echo && equal_CRUD && equal_health);
      }
    }

    void testGetPort(int expected_result) {
      int port = out_config.GetPort();
      EXPECT_TRUE(port == expected_result);
    }

    void testToString(std::string expected, int depth) {
      std::string res = out_config.ToString(depth);
      std::cout << "Res is: " << std::endl;
      std::cout << res << std::endl;
      EXPECT_TRUE(expected == res);
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
  std::unordered_map<std::string, std::string> expected_CRUD( {} );
  std::unordered_set<std::string> expected_health( {} );                                                       
  testGetPaths(expected_static, expected_echo, expected_CRUD, expected_health, true);
}

TEST_F(ConfigParserTest, EndpointsRedefinedPathConfig) {
  testParsing("endpoints_redefined_path_config", true);
  std::unordered_map<std::string, std::string> expected_static( {{ "/static", "../static/" }});
  std::unordered_set<std::string> expected_echo( { "/echo", "/echo1" } );
  std::unordered_map<std::string, std::string> expected_CRUD( {} );
  std::unordered_set<std::string> expected_health( {} );
  testGetPaths(expected_static, expected_echo, expected_CRUD, expected_health, true);
}

/*  Wrong Test Cases for Endpoints Parsing */

TEST_F(ConfigParserTest, EndpointsEchoFewArgumentsConfig) {
  testParsing("endpoints_echo_few_arguments_config", true);
  std::unordered_map<std::string, std::string> expected_static( {{ "/static", "../static/" },
                                                                 { "/static1", "../tests/static_files/"}} );
  std::unordered_set<std::string> expected_echo( { "/echo", "/echo1" } );
  std::unordered_map<std::string, std::string> expected_CRUD( {} );
  std::unordered_set<std::string> expected_health( {} );
  testGetPaths(expected_static, expected_echo, expected_CRUD, expected_health, false);
}

TEST_F(ConfigParserTest, EndpointsStaticFewArgumentsConfig) {
  testParsing("endpoints_static_few_arguments_config", true);
  std::unordered_map<std::string, std::string> expected_static( {{ "/static", "../static/" },
                                                                 { "/static1", "../tests/static_files/"}} );
  std::unordered_set<std::string> expected_echo( { "/echo", "/echo1" } );
  std::unordered_map<std::string, std::string> expected_CRUD( {} );
  std::unordered_set<std::string> expected_health( {} );
  testGetPaths(expected_static, expected_echo, expected_CRUD, expected_health, false);
}

TEST_F(ConfigParserTest, CRUDConfig) {
  testParsing("CRUD_config", true);
  std::unordered_map<std::string, std::string> expected_static( {} );
  std::unordered_set<std::string> expected_echo( {} );
  std::unordered_map<std::string, std::string> expected_CRUD( {{ "/api", "../crud"}} );
  std::unordered_set<std::string> expected_health( {} );
  testGetPaths(expected_static, expected_echo, expected_CRUD, expected_health, true);
}

TEST_F(ConfigParserTest, healthConfig) {
  testParsing("health_config", true);
  std::unordered_map<std::string, std::string> expected_static( {} );
  std::unordered_set<std::string> expected_echo( {} );
  std::unordered_map<std::string, std::string> expected_CRUD( {} );
  std::unordered_set<std::string> expected_health( { "/health" } );
  testGetPaths(expected_static, expected_echo, expected_CRUD, expected_health, true);
}

TEST_F(ConfigParserTest, PortCorrectConfig) {
  testParsing("port_correct_config", true);
  testGetPort(80);
}


TEST_F(ConfigParserTest, PortInvalidConfig) {
  testParsing("port_invalid_config", true);
  testGetPort(-1);
}

TEST_F(ConfigParserTest, ToStringExampleConfig) {
  testParsing("example_config", true);
  std::ifstream t(TEST_CONFIG_PATH + "example_config");
  std::stringstream buffer;
  buffer << t.rdbuf();

  std::cout << "the buffer result is: " << std::endl;
  std::cout << buffer.str() << std::endl;
  testToString(buffer.str(), 1);
}
