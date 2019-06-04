// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#include "Utilities.hpp"

#include <string.h>
#include <fstream>
#include <iostream>

#include <boost/algorithm/string.hpp>

#include "AMOCommand.hpp"

const int kMaxConfigLineLength = 255;

std::vector<std::tuple<std::string, std::string, std::string>>
    Utilities::ReadConfig(const std::string &config_path) {
  // Read list of host names and ports from config file.
  std::vector<std::tuple<std::string,
    std::string, std::string>> server_addresses;

  std::ifstream config_file(config_path);
  if (!config_file) {
    std::cerr << "cannot open config file" << std::endl;
    return {};
  }

  char buf[kMaxConfigLineLength];
  while (config_file) {
    config_file.getline(buf, kMaxConfigLineLength);
    std::string line(buf);
    if (config_file) {
      int pos = line.find_first_of(' ');
      auto host = line.substr(0, pos);
      auto port_name = line.substr(pos + 1);
      pos = port_name.find_first_of(' ');
      auto port = port_name.substr(0, pos);
      auto name = port_name.substr(pos + 1);
      server_addresses.push_back(
        std::make_tuple(host.c_str(), port.c_str(), name.c_str()));
    }
  }

  return server_addresses;
}

std::vector<KVStore::AMOCommand> Utilities::ParseOperations(
    char *unparsed_ops) {
  std::vector<std::string> str_ops;
  boost::split(str_ops, unparsed_ops, boost::is_any_of(","));

  std::vector<KVStore::AMOCommand> ops;
  for (int i = 0; i < str_ops.size(); i++) {
      std::vector<std::string> op_parts;
      boost::split(op_parts, str_ops[i], boost::is_any_of(" "));
      std::string action_str = op_parts[0];
      std::string key = op_parts[1];
      std::string value = "";
      KVStore::Action action = KVStore::Action::kGet;

      if (action_str.compare("PUT") == 0) {
        value = op_parts[2];
        action = KVStore::Action::kPut;
      } else if (action_str.compare("GET") == 0) {
        action = KVStore::Action::kGet;
      } else if (action_str.compare("APPEND") == 0) {
        action = KVStore::Action::kAppend;
      } else {
        action = KVStore::Action::kNoOp;
      }

      KVStore::AMOCommand op(i, key, value, action);
      ops.push_back(op);
  }

  return ops;
}

void Utilities::DebugPrint(std::string s) {
  if (kDebugPrint) {
    std::cout << s << std::endl;
  }
}
