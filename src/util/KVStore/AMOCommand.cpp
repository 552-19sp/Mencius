// Copyright 2019 Jack Khuu, Justin Johnson, Lukas Joswiak

#include "AMOCommand.hpp"

#include <string>

#include "Action.hpp"

namespace KVStore {
using std::string;

AMOCommand::AMOCommand(int seq_num, string key, string value, Action action):
            seq_num_(seq_num), key_(key), value_(value), action_(action) {}

bool AMOCommand::operator< (const AMOCommand &r) const {
  return this->seq_num_ < r.seq_num_;
}

int AMOCommand::GetSeqNum() const {
  return seq_num_;
}

string AMOCommand::GetKey() const {
  return key_;
}

string AMOCommand::GetValue() const {
  return value_;
}

Action AMOCommand::GetAction() const {
  return action_;
}

}  // namespace KVStore
