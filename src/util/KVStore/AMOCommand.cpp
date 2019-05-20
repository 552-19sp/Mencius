#include "./AMOCommand.h"

#include "./Action.h"

namespace AMOCommand {
using namespace std;

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

int AMOCommand::getSeqNum() const {
  return seqNum_;
}

int AMOCommand::getKey() const {
  return key_;
}

string AMOCommand::getValue() const {
  return value_;
}

Action::Action AMOCommand::getAction() const {
  return action_;
}
} // namespace AMOCommand 
