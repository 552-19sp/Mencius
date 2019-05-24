// Copyright 2019 Jack Khuu, Justin Johnson, Lukas Joswiak

#ifndef INCLUDE_KVSTORE_HPP_
#define INCLUDE_KVSTORE_HPP_

#include <map>
#include <string>

namespace KVStore {

class KVStore {
 public:
  std::string Put(const std::string &key, const std::string &value);
  std::string Append(const std::string &key, const std::string &value);
  std::string Get(const std::string &key);

 private:
  std::map<std::string, std::string> store_;
};
}  // namespace KVStore

#endif  // INCLUDE_KVSTORE_HPP_
