// Copyright 2019 Jack Khuu, Justin Johnson, Lukas Joswiak

#include "AMOCommand.hpp"

#include <string>

#include "Action.hpp"

namespace KVStore {
using std::string;

AMOCommand::AMOCommand(int seq_num, const string &key, const string &value,
                        const Action &action):
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

std::string AMOCommand::Encode() const {
  std::ostringstream archive_stream;
  boost::archive::text_oarchive oa(archive_stream);
  oa << *this;
  return archive_stream.str();
}

AMOCommand AMOCommand::Decode(const std::string data) {
  AMOCommand c;
  std::istringstream archive_stream(data);
  boost::archive::text_iarchive archive(archive_stream);
  archive >> c;
  return c;
}
}  // namespace KVStore
