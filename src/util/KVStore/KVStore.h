#ifndef _KVSTORE_H_
#define _KVSTORE_H_

#include <map>
#include <string>

namespace KVStore {
using namespace std;

class KVStore {
  public:
    string insert(const int &key, const string &value);
    string append(const int &key, const string &value);
    string get(const int &key);

  private:
    map<int, string> store_;
};
} // namespace KVStore

#endif // _KVSTORE_H_
