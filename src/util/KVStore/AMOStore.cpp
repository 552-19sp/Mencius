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

bool AMOStore::alreadyExecuted(const AMOCommand::AMOCommand &command) const {
  return prev_.find(command) != prev_.end();
}

AMOStore::AMOStore() {
  kvStore_ = KVStore::KVStore();
}
      
AMOResponse::AMOResponse AMOStore::execute(const AMOCommand::AMOCommand &command) {
  string s;
  if (!alreadyExecuted(command)) {
    cout << "New" << endl;
    switch(command.getAction()) {
      case PUT:
        cout << "PUT" << endl;
        s = kvStore_.insert(command.getKey(), command.getValue());
        break;
      case APPEND:
        cout << "APPEND" << endl;
        s = kvStore_.append(command.getKey(), command.getValue());
        break;
      case GET:
        cout << "GET" << endl;
        s = kvStore_.get(command.getKey());
        break;
      default:  
        exit(EXIT_FAILURE);
    }

    prev_[command] = AMOResponse::AMOResponse(command, s);
  }
  cout << "Returned from Execute: ";
  cout << prev_[command].getValue() << endl;
  return prev_[command];
}
} //namespace AMOStore
