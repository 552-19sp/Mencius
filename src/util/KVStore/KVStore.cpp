// Copyright 2019 Jack Khuu, Justin Johnson, Lukas Joswiak

#include "KVStore.hpp"

#include <stdlib.h>

#include <map>
#include <string>

namespace KVStore {
using std::map;
using std::string;

string KVStore::Put(const string &key, const string &value) {
  if (store_.find(key) != store_.end()) {
    // TODO(jackkhuu): Support an Error Message
    exit(EXIT_FAILURE);
  }
  store_[key] = value;

  return store_[key];
}

string KVStore::Append(const string &key, const string &value) {
  if (store_.find(key) == store_.end()) {
    // TODO(jackkhuu): Support an Error Message
    exit(EXIT_FAILURE);
  }
  store_[key] = store_[key] + value;
  return store_[key];
}

string KVStore::Get(const string &key) {
  return store_[key];
}
}  // namespace KVStore
