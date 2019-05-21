// Copyright 2019 Jack Khuu, Justin Johnson, Lukas Joswiak

#include "AMOStore.hpp"

#include <stdlib.h>

#include <map>
#include <string>
#include <iostream>

#include "Action.hpp"
#include "AMOCommand.hpp"
#include "AMOResponse.hpp"
#include "KVStore.hpp"

namespace AMOStore {
using std::string;
using std::map;
using std::cout;
using std::endl;

bool AMOStore::AlreadyExecuted(const AMOCommand::AMOCommand &command) const {
  return prev_.find(command) != prev_.end();
}

AMOStore::AMOStore() {
  kvStore_ = KVStore::KVStore();
}

AMOResponse::AMOResponse AMOStore::Execute(
                const AMOCommand::AMOCommand &command) {
  string s;
  if (!AlreadyExecuted(command)) {
    // cout << "New" << endl;
    switch (command.GetAction()) {
      case Action::PUT:
        // cout << "PUT" << endl;
        s = kvStore_.Insert(command.GetKey(), command.GetValue());
        break;
      case Action::APPEND:
        // cout << "APPEND" << endl;
        s = kvStore_.Append(command.GetKey(), command.GetValue());
        break;
      case Action::GET:
        // cout << "GET" << endl;
        s = kvStore_.Get(command.GetKey());
        break;
      default:
        exit(EXIT_FAILURE);
    }

    prev_[command] = AMOResponse::AMOResponse(command, s);
  }
  // cout << "Returned from Execute: ";
  // cout << prev_[command].GetValue() << endl;
  return prev_[command];
}
}  // namespace AMOStore
