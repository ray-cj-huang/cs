// An nginx config file parser.
//
// See:
//   http://wiki.nginx.org/Configuration
//   http://blog.martinfjordvald.com/2010/07/nginx-primer/
//
// How Nginx does it:
//   http://lxr.nginx.org/source/src/core/ngx_conf_file.c

#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <stack>
#include <string>
#include <vector>
#include <unordered_map>

#include "config_parser.h"
#include "logger.h"

const int ENDPOINT_MIN_SIZE = 3;
const int STATIC_ENDPOINT_SIZE = 3;
const std::string ENDPOINT = "location";
const std::string STATIC_NAME = "StaticHandler";
const std::string ECHO_NAME = "EchoHandler";
const std::string CRUD_NAME = "CRUDHandler";
const std::string ROOT = "root";

// To get the port number from config
int NginxConfig::GetPort() {
  // Traverse statements
  for (auto singleStatement : statements_) {
    if (singleStatement->child_block_.get() == nullptr) { // No child block
      if (singleStatement->tokens_.size() == 2 && singleStatement->tokens_[0] == "port") {
        int portNum = atoi(singleStatement->tokens_[1].c_str()); // Convert string port number to int
        if (portNum >= 0 && portNum <= 0xffff) // Check for valid port number
          return portNum;
        else
          return -1;
      }
    } else { // Traverse child blocks as well
      int portNum = singleStatement->child_block_->GetPort();
      if (portNum != -1)
        return portNum;
    }
  }
  return -1;
}

void NginxConfig::GetPaths(std::unordered_map<std::string, std::string> &static_paths,
                           std::unordered_set<std::string> &echo_paths,
                           std::unordered_map<std::string, std::string> &CRUD_paths) {

  for (auto singleStatement : statements_) {
    if (singleStatement->tokens_.size() && singleStatement->tokens_[0] == ENDPOINT && singleStatement->tokens_.size() >= ENDPOINT_MIN_SIZE) {
      
      std::string endpoint = singleStatement->tokens_[1];
      std::string endpoint_type = singleStatement->tokens_[2];

      if (endpoint_type == STATIC_NAME) {
        if (singleStatement->tokens_.size() >= STATIC_ENDPOINT_SIZE) {
          if (singleStatement->child_block_) {
            for (auto childStatement : singleStatement->child_block_->statements_) {
              if (childStatement->tokens_.size() >= 2 && childStatement->tokens_[0] == ROOT) {
                std::string directory = childStatement->tokens_[1];
                static_paths.insert( {{ endpoint, directory }});
                Logger::logInfo("Static path " + endpoint + " successfully parsed.");
              }
            }
          }
        }
        else {
          Logger::logWarning("When specifying a static endpoint in the config file, you need to specify both the endpoint and the path.");
        }
        
      }
      else if (endpoint_type == ECHO_NAME) {
        echo_paths.insert(endpoint);
        Logger::logInfo("Echo path " + endpoint + " successfully parsed.");
      }
      else if (endpoint_type == CRUD_NAME) {
        if (singleStatement->tokens_.size() >= STATIC_ENDPOINT_SIZE) {
          if (singleStatement->child_block_) {
            for (auto childStatement : singleStatement->child_block_->statements_) {
              if (childStatement->tokens_.size() >= 2 && childStatement->tokens_[0] == ROOT) {
                std::string directory = childStatement->tokens_[1];
                CRUD_paths.insert( {{ endpoint, directory }});
                Logger::logInfo("CRUD path " + endpoint + " successfully parsed.");
              }
            }
          }
        }
        else {
          Logger::logWarning("When specifying a CRUD endpoint in the config file, you need to specify both the endpoint and the path.");
        }
        
      }

    }

    if (singleStatement->child_block_.get() != nullptr) {
      singleStatement->child_block_->GetPaths(static_paths, echo_paths, CRUD_paths);
    }
  }
}

std::string NginxConfig::ToString(int depth) {
  std::string serialized_config;
  for (const auto& statement : statements_) {
    serialized_config.append(statement->ToString(depth));
  }
  return serialized_config;
}

std::string NginxConfigStatement::ToString(int depth) {
  std::string serialized_statement;
  for (int i = 0; i < depth; ++i) {
    serialized_statement.append("  ");
  }
  for (unsigned int i = 0; i < tokens_.size(); ++i) {
    if (i != 0) {
      serialized_statement.append(" ");
    }
    serialized_statement.append(tokens_[i]);
  }
  if (child_block_.get() != nullptr) {
    serialized_statement.append(" {\n");
    serialized_statement.append(child_block_->ToString(depth + 1));
    for (int i = 0; i < depth; ++i) {
      serialized_statement.append("  ");
    }
    serialized_statement.append("}");
  } else {
    serialized_statement.append(";");
  }
  serialized_statement.append("\n");
  return serialized_statement;
}

const char* NginxConfigParser::TokenTypeAsString(TokenType type) {
  switch (type) {
    case TOKEN_TYPE_START:         return "TOKEN_TYPE_START";
    case TOKEN_TYPE_NORMAL:        return "TOKEN_TYPE_NORMAL";
    case TOKEN_TYPE_START_BLOCK:   return "TOKEN_TYPE_START_BLOCK";
    case TOKEN_TYPE_END_BLOCK:     return "TOKEN_TYPE_END_BLOCK";
    case TOKEN_TYPE_COMMENT:       return "TOKEN_TYPE_COMMENT";
    case TOKEN_TYPE_STATEMENT_END: return "TOKEN_TYPE_STATEMENT_END";
    case TOKEN_TYPE_EOF:           return "TOKEN_TYPE_EOF";
    case TOKEN_TYPE_ERROR:         return "TOKEN_TYPE_ERROR";
    default:                       return "Unknown token type";
  }
}

NginxConfigParser::TokenType NginxConfigParser::ParseToken(std::istream* input,
                                                           std::string* value) {
  TokenParserState state = TOKEN_STATE_INITIAL_WHITESPACE;
  while (input->good()) {
    const char c = input->get();
    if (!input->good()) {
      break;
    }
    switch (state) {
      case TOKEN_STATE_INITIAL_WHITESPACE:
        switch (c) {
          case '{':
            *value = c;
            return TOKEN_TYPE_START_BLOCK;
          case '}':
            *value = c;
            return TOKEN_TYPE_END_BLOCK;
          case '#':
            *value = c;
            state = TOKEN_STATE_TOKEN_TYPE_COMMENT;
            continue;
          case '"':
            *value = c;
            state = TOKEN_STATE_DOUBLE_QUOTE;
            continue;
          case '\'':
            *value = c;
            state = TOKEN_STATE_SINGLE_QUOTE;
            continue;
          case ';':
            *value = c;
            return TOKEN_TYPE_STATEMENT_END;
          case ' ':
          case '\t':
          case '\n':
          case '\r':
            continue;
          default:
            *value += c;
            state = TOKEN_STATE_TOKEN_TYPE_NORMAL;
            continue;
        }
      case TOKEN_STATE_SINGLE_QUOTE:
        // The end of a quoted token should be followed by whitespace.
        // Allow for backslash-escaping within strings.
        *value += c;
        if (c == '\'') {
          // the end of a quoted token should be followed by whitespace.
          const char c_next = input->get();
          if (c_next == ' ' || c_next == '\t' || c_next == '\n' || 
              c_next == ';' || c_next == '{' || c_next == '}') {
            input->unget();
          } else {
            return TOKEN_TYPE_ERROR;
          }
          return TOKEN_TYPE_NORMAL;
        } else if (c == '\\') { // escaped
          value -> pop_back();
          state = TOKEN_STATE_SINGLE_QUOTE_ESCAPED;
        }
        continue;
      case TOKEN_STATE_DOUBLE_QUOTE:
        *value += c;
        if (c == '"') {
          // the end of a quoted token should be followed by whitespace.
          const char c_next = input->get();
          if (c_next == ' ' || c_next == '\t' || c_next == '\n' || 
              c_next == ';' || c_next == '{' || c_next == '}') {
            input->unget();
          } else {
            return TOKEN_TYPE_ERROR;
          }
          return TOKEN_TYPE_NORMAL;
        } else if (c == '\\') { // escaped
          value -> pop_back();
          state = TOKEN_STATE_DOUBLE_QUOTE_ESCAPED;
        }
        continue;
      case TOKEN_STATE_TOKEN_TYPE_COMMENT:
        if (c == '\n' || c == '\r') {
          return TOKEN_TYPE_COMMENT;
        }
        *value += c;
        continue;
      case TOKEN_STATE_TOKEN_TYPE_NORMAL:
        if (c == ' ' || c == '\t' || c == '\n' || c == '\t' ||
            c == ';' || c == '{' || c == '}') {
          input->unget();
          return TOKEN_TYPE_NORMAL;
        }
        *value += c;
        continue;
      case TOKEN_STATE_SINGLE_QUOTE_ESCAPED: // escape
        *value += c;
        state = TOKEN_STATE_SINGLE_QUOTE;
        continue;
      case TOKEN_STATE_DOUBLE_QUOTE_ESCAPED: // escape
        *value += c;
        state = TOKEN_STATE_DOUBLE_QUOTE;
        continue;
    }
  }

  // If we get here, we reached the end of the file.
  if (state == TOKEN_STATE_SINGLE_QUOTE ||
      state == TOKEN_STATE_DOUBLE_QUOTE) {
    return TOKEN_TYPE_ERROR;
  }

  return TOKEN_TYPE_EOF;
}

bool NginxConfigParser::Parse(std::istream* config_file, NginxConfig* config) {
  std::stack<NginxConfig*> config_stack;
  config_stack.push(config);
  TokenType last_token_type = TOKEN_TYPE_START;
  TokenType token_type;
  while (true) {
    std::string token;
    token_type = ParseToken(config_file, &token);
    printf ("%s: %s\n", TokenTypeAsString(token_type), token.c_str());
    if (token_type == TOKEN_TYPE_ERROR) {
      break;
    }

    if (token_type == TOKEN_TYPE_COMMENT) {
      // Skip comments.
      continue;
    }

    if (token_type == TOKEN_TYPE_START) {
      // Error.
      break;
    } else if (token_type == TOKEN_TYPE_NORMAL) {
      if (last_token_type == TOKEN_TYPE_START ||
          last_token_type == TOKEN_TYPE_STATEMENT_END ||
          last_token_type == TOKEN_TYPE_START_BLOCK ||
          last_token_type == TOKEN_TYPE_END_BLOCK ||
          last_token_type == TOKEN_TYPE_NORMAL) {
        if (last_token_type != TOKEN_TYPE_NORMAL) {
          config_stack.top()->statements_.emplace_back(
              new NginxConfigStatement);
        }
        config_stack.top()->statements_.back().get()->tokens_.push_back(
            token);
      } else {
        // Error.
        break;
      }
    } else if (token_type == TOKEN_TYPE_STATEMENT_END) {
      if (last_token_type != TOKEN_TYPE_NORMAL) {
        // Error.
        break;
      }
    } else if (token_type == TOKEN_TYPE_START_BLOCK) {
      if (last_token_type != TOKEN_TYPE_NORMAL) {
        // Error.
        break;
      }
      NginxConfig* const new_config = new NginxConfig;
      config_stack.top()->statements_.back().get()->child_block_.reset(
          new_config);
      config_stack.push(new_config);
    } else if (token_type == TOKEN_TYPE_END_BLOCK) {
      if (last_token_type != TOKEN_TYPE_STATEMENT_END &&
          last_token_type != TOKEN_TYPE_START_BLOCK &&  // {} is valid 
          last_token_type != TOKEN_TYPE_END_BLOCK) {    // }} is valid
        // Error.
        break;
      }
      config_stack.pop();
    } else if (token_type == TOKEN_TYPE_EOF) {
       if (last_token_type != TOKEN_TYPE_STATEMENT_END &&
          last_token_type != TOKEN_TYPE_END_BLOCK &&
          last_token_type != TOKEN_TYPE_START) {  // empty config is valid
        // Error.
        break;
      } else if (config_stack.size() > 1) {
        break;
      }
      return true;
    } else {
      // Error. Unknown token.
      break;
    }
    last_token_type = token_type;
  }

  printf ("Bad transition from %s to %s\n",
          TokenTypeAsString(last_token_type),
          TokenTypeAsString(token_type));
  return false;
}

bool NginxConfigParser::Parse(const char* file_name, NginxConfig* config) {
  std::ifstream config_file;
  config_file.open(file_name);
  if (!config_file.good()) {
    printf ("Failed to open config file: %s\n", file_name);
    return false;
  }

  const bool return_value =
      Parse(dynamic_cast<std::istream*>(&config_file), config);
  config_file.close();
  return return_value;
}
