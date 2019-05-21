// Copyright 2019 Jack Khuu, Justin Johnson, Lukas Joswiak

#include "AMOCommand.hpp"

#include <string>

#include "Action.hpp"

namespace AMOCommand {
using std::string;

AMOCommand::AMOCommand(const int &seqNum, const int &key, const string &value,
                        const Action::Action &action) {
  seqNum_ = seqNum;
  key_ = key;
  value_ = value;
  action_ = action;
}

bool AMOCommand::operator< (const AMOCommand &r) const {
  return this->seqNum_ < r.seqNum_;
}

int AMOCommand::GetSeqNum() const {
  return seqNum_;
}

int AMOCommand::GetKey() const {
  return key_;
}

string AMOCommand::GetValue() const {
  return value_;
}

Action::Action AMOCommand::GetAction() const {
  return action_;
}
}  // namespace AMOCommand
