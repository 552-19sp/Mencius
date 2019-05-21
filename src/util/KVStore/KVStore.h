#ifndef _KVSTORE_H_
#define _KVSTORE_H_

#include <map>
#include <string>

namespace KVStore {
using namespace std;

class KVStore {
  public:
    string Insert(const int &key, const string &value);
    string Append(const int &key, const string &value);
    string Get(const int &key);

  private:
    map<int, string> store_;
};
} // namespace KVStore

#endif // _KVSTORE_H_
