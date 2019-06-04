// Copyright 2019 Jack Khuu, Justin Johnson, Lukas Joswiak

#include "AMOCommand.hpp"

#include <iostream>

#include "Action.hpp"

namespace KVStore {
using std::string;

AMOCommand::AMOCommand()
    : seq_num_(-1),
      key_(""),
      value_(""),
      action_(KVStore::Action::kNoOp) {}

AMOCommand::AMOCommand(int seq_num, string key, string value, Action action)
    : seq_num_(seq_num),
      key_(key),
      value_(value),
      action_(action) {}

AMOCommand::AMOCommand(const AMOCommand &command) {
  seq_num_ = command.seq_num_;
  key_ = command.key_;
  value_ = command.value_;
  action_ = command.action_;
}

AMOCommand::~AMOCommand() {
  // std::cout << "AMOCommand destructor called" << std::endl;
}

bool AMOCommand::operator< (const AMOCommand &r) const {
  return this->seq_num_ < r.seq_num_;
}

bool AMOCommand::operator== (const AMOCommand &r) const {
  return this->seq_num_ == r.seq_num_ &&
      this->key_.compare(r.key_) == 0 &&
      this->value_.compare(r.value_) == 0 &&
      this->action_ == r.action_;
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
