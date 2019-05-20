# include <stdlib.h>

# include "./KVStore.h"

namespace KVStore {
using namespace std;

string KVStore::insert(const int &key, const string &value) {
  if (store_.find(key) != store_.end()) {
    // TODO: Support an Error Message
    exit(EXIT_FAILURE);
  }
  store_[key] = value;
      
  return store_[key];
}

string KVStore::append(const int &key, const string &value) {
  if (store_.find(key) == store_.end()) {
    // TODO: Support an Error Message
    exit(EXIT_FAILURE);
  }
  store_[key] = store_[key] + value;  
  return store_[key];
}

string KVStore::get(const int &key) {
  return store_[key];
}
} //namespace KVStore
