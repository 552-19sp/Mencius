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

AMOStore::AMOStore():kvStore_(KVStore::KVStore()) {}

bool AMOStore::AlreadyExecuted(const AMOCommand::AMOCommand &command) const {
  return prev_.find(command) != prev_.end();
}

AMOResponse::AMOResponse AMOStore::Execute(
                const AMOCommand::AMOCommand &command) {
  string s;
  if (!AlreadyExecuted(command)) {
    switch (command.GetAction()) {
      case Action::PUT:
        s = kvStore_.Put(command.GetKey(), command.GetValue());
        break;
      case Action::APPEND:
        s = kvStore_.Append(command.GetKey(), command.GetValue());
        break;
      case Action::GET:
        s = kvStore_.Get(command.GetKey());
        break;
      default:
        exit(EXIT_FAILURE);
    }

    prev_[command] = AMOResponse::AMOResponse(command, s);
  }

  return prev_[command];
}
}  // namespace AMOStore
