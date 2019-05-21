// Copyright 2019 Jack Khuu, Justin Johnson, Lukas Joswiak

#ifndef INCLUDE_AMOCOMMAND_HPP_
#define INCLUDE_AMOCOMMAND_HPP_

#include <string>

#include "Action.hpp"

namespace AMOCommand {
using std::string;

class AMOCommand {
 public:
  AMOCommand() {}
  AMOCommand(const int &seqNum, const int &key, const string &value,
                   const Action::Action &action);

  bool operator< (const AMOCommand &r) const;

  int GetSeqNum() const;
  int GetKey() const;
  string GetValue() const;
  Action::Action GetAction() const;

 private:
  int seqNum_;
  int key_;
  string value_;
  Action::Action action_;
};
}  // namespace AMOCommand

#endif  // INCLUDE_AMOCOMMAND_HPP_
