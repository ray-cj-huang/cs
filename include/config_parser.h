// An nginx config file parser.

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>

class NginxConfig;

// The parsed representation of a single config statement.
class NginxConfigStatement {
 public:
  std::string ToString(int depth);
  std::vector<std::string> tokens_;
  std::unique_ptr<NginxConfig> child_block_;
};

// The parsed representation of the entire config.
class NginxConfig {
 public:
  std::string ToString(int depth = 0);
  std::vector<std::shared_ptr<NginxConfigStatement>> statements_;
  int GetPort(); // Get Port #
  std::unordered_set<std::string> echo_paths_;
  std::unordered_map<std::string, std::string> static_paths_;
  std::unordered_map<std::string, std::string> CRUD_paths_;
  std::unordered_set<std::string> health_paths_;
  std::unordered_set<std::string> sleep_paths_;
  std::unordered_map<std::string, std::string> caption_this_paths_;
  void GetPaths(std::unordered_map<std::string, std::string> &static_paths,
                std::unordered_set<std::string> &echo_paths,
                std::unordered_map<std::string, std::string> &CRUD_paths,
                std::unordered_set<std::string> &health_paths,
                std::unordered_set<std::string> &sleep_paths,
                std::unordered_map<std::string, std::string> &caption_this_paths);


};

// The driver that parses a config file and generates an NginxConfig.
class NginxConfigParser {
 public:
  NginxConfigParser() {} // GCOVR_EXCL_LINE

  // Take a opened config file or file name (respectively) and store the
  // parsed config in the provided NginxConfig out-param.  Returns true
  // iff the input config file is valid.
  bool Parse(std::istream* config_file, NginxConfig* config);
  bool Parse(const char* file_name, NginxConfig* config);

 private:
  enum TokenType {
    TOKEN_TYPE_START = 0,
    TOKEN_TYPE_NORMAL = 1,
    TOKEN_TYPE_START_BLOCK = 2,
    TOKEN_TYPE_END_BLOCK = 3,
    TOKEN_TYPE_COMMENT = 4,
    TOKEN_TYPE_STATEMENT_END = 5,
    TOKEN_TYPE_EOF = 6,
    TOKEN_TYPE_ERROR = 7,
  };
  const char* TokenTypeAsString(TokenType type);

  enum TokenParserState {
    TOKEN_STATE_INITIAL_WHITESPACE = 0,
    TOKEN_STATE_SINGLE_QUOTE = 1,
    TOKEN_STATE_DOUBLE_QUOTE = 2,
    TOKEN_STATE_TOKEN_TYPE_COMMENT = 3,
    TOKEN_STATE_TOKEN_TYPE_NORMAL = 4,
    TOKEN_STATE_TOKEN_TYPE_QUOTED_TOKEN = 5,
    TOKEN_STATE_SINGLE_QUOTE_ESCAPED = 6,
    TOKEN_STATE_DOUBLE_QUOTE_ESCAPED = 7
  };

  TokenType ParseToken(std::istream* input, std::string* value);
};
