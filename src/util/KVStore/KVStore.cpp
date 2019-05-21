#include "./KVStore.h"

#include <map>
#include <string>

#include <stdlib.h>

namespace KVStore {
using namespace std;

string KVStore::Insert(const int &key, const string &value) {
  if (store_.find(key) != store_.end()) {
    // TODO: Support an Error Message
    exit(EXIT_FAILURE);
  }
  store_[key] = value;
      
  return store_[key];
}

string KVStore::Append(const int &key, const string &value) {
  if (store_.find(key) == store_.end()) {
    // TODO: Support an Error Message
    exit(EXIT_FAILURE);
  }
  store_[key] = store_[key] + value;  
  return store_[key];
}

string KVStore::Get(const int &key) {
  return store_[key];
}
} //namespace KVStore
