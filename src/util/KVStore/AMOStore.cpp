#include "./AMOStore.h"

#include <map>
#include <string>
#include <iostream>
#include <stdlib.h>

#include "./Action.h"
#include "./AMOCommand.h"
#include "./AMOResponse.h"
#include "./KVStore.h"

namespace AMOStore {
using namespace std;
using namespace Action;

bool AMOStore::AlreadyExecuted(const AMOCommand::AMOCommand &command) const {
  return prev_.find(command) != prev_.end();
}

AMOStore::AMOStore() {
  kvStore_ = KVStore::KVStore();
}
      
AMOResponse::AMOResponse AMOStore::Execute(const AMOCommand::AMOCommand &command) {
  string s;
  if (!AlreadyExecuted(command)) {
    cout << "New" << endl;
    switch(command.GetAction()) {
      case PUT:
        cout << "PUT" << endl;
        s = kvStore_.Insert(command.GetKey(), command.GetValue());
        break;
      case APPEND:
        cout << "APPEND" << endl;
        s = kvStore_.Append(command.GetKey(), command.GetValue());
        break;
      case GET:
        cout << "GET" << endl;
        s = kvStore_.Get(command.GetKey());
        break;
      default:  
        exit(EXIT_FAILURE);
    }

    prev_[command] = AMOResponse::AMOResponse(command, s);
  }
  cout << "Returned from Execute: ";
  cout << prev_[command].GetValue() << endl;
  return prev_[command];
}
} //namespace AMOStore
