// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#ifndef INCLUDE_UTILITIES_HPP_
#define INCLUDE_UTILITIES_HPP_

#include <string>
#include <tuple>
#include <vector>

#include "AMOCommand.hpp"

class Utilities {
 private:
  // Flag for enabling debug printing
  static const bool kDebugPrint = true;

 public:
  static std::vector<std::tuple<std::string,
    std::string, std::string>> ReadConfig(const std::string &config_path);

  // Given a string of comma separated operations,
  // parses the operations and returns each operation
  // separated into a vector.
  static std::vector<KVStore::AMOCommand> ParseOperations(char *unparsed_ops);

  // Given a string, prints the string delimited with a new line
  // if kDebugPrint flag is enabled
  static void DebugPrint(std::string s);
};

#endif  // INCLUDE_UTILITIES_HPP_
