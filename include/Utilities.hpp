// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#ifndef INCLUDE_UTILITIES_HPP_
#define INCLUDE_UTILITIES_HPP_

#include <string>
#include <tuple>
#include <vector>

class Utilities {
 public:
  static std::vector<std::tuple<std::string, std::string>> ReadConfig(
    const std::string &config_path);
};

#endif  // INCLUDE_UTILITIES_HPP_
