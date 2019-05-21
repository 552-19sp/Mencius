// Copyright 2019 Jack Khuu, Justin Johnson, Lukas Joswiak

#ifndef INCLUDE_KVSTORE_HPP_
#define INCLUDE_KVSTORE_HPP_

#include <map>
#include <string>

namespace KVStore {
using std::string;
using std::map;

class KVStore {
 public:
  string Insert(const int &key, const string &value);
  string Append(const int &key, const string &value);
  string Get(const int &key);

 private:
  map<int, string> store_;
};
}  // namespace KVStore

#endif  // INCLUDE_KVSTORE_HPP_
