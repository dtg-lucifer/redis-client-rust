#include "resp_encoder.hpp"

#include <sstream>

namespace resp {
std::string encode_bulk_string(const std::string& str) {
  std::ostringstream oss;
  oss << "$" << str.length() << "\r\n" << str << "\r\n";
  return oss.str();
}

std::string encode_array(const std::vector<std::string>& elements) {
  std::ostringstream oss;
  oss << "*" << elements.size() << "\r\n";
  for (const auto& element : elements) {
    oss << encode_bulk_string(element);
  }
  return oss.str();
}

std::vector<std::string> tokenize(const std::string& input) {
  std::vector<std::string> tokens;
  std::istringstream iss(input);
  std::string token;

  bool in_quotes = false;
  std::string quoted_token;

  while (iss >> token) {
    // Check if token starts with a quote
    if (!in_quotes && token.front() == '"') {
      in_quotes = true;
      quoted_token = token.substr(1);  // Remove leading quote

      // If it also ends with a quote, it's a complete quoted token
      if (token.length() > 1 && token.back() == '"') {
        in_quotes = false;
        quoted_token = quoted_token.substr(
            0, quoted_token.length() - 1);  // Remove trailing quote
        tokens.push_back(quoted_token);
      }
    }
    // Check if token ends with a quote
    else if (in_quotes && token.back() == '"') {
      in_quotes = false;
      quoted_token +=
          " " + token.substr(0, token.length() - 1);  // Remove trailing quote
      tokens.push_back(quoted_token);
    }
    // If we're in the middle of a quoted string
    else if (in_quotes) {
      quoted_token += " " + token;
    }
    // Regular token, not part of a quoted string
    else {
      tokens.push_back(token);
    }
  }

  // If we're still in quotes after processing all tokens,
  // add what we have (handles unclosed quotes)
  if (in_quotes) {
    tokens.push_back(quoted_token);
  }

  return tokens;
}

std::string encode_command(const std::string& cmd,
                           const std::vector<std::string>& args) {
  std::vector<std::string> all_elements;
  all_elements.push_back(cmd);
  all_elements.insert(all_elements.end(), args.begin(), args.end());

  return encode_array(all_elements);
}

std::string encode_raw_command(const std::string& raw_cmd) {
  std::vector<std::string> tokens = tokenize(raw_cmd);

  if (tokens.empty()) {
    return "";
  }

  std::string cmd = tokens[0];
  std::vector<std::string> args(tokens.begin() + 1, tokens.end());

  return encode_command(cmd, args);
}

}  // namespace resp
