// Copyright 2019 Jack Khuu, Justin Johnson, Lukas Joswiak

#include "AMOResponse.hpp"

#include <string>

#include "AMOCommand.hpp"

namespace KVStore {
using std::string;

AMOResponse::AMOResponse() {}

AMOResponse::AMOResponse(const AMOCommand &command,
                     const string &value):command_(command), value_(value) {}

bool AMOResponse::operator< (const AMOResponse &right) const {
  return command_ < right.command_;
}

AMOCommand AMOResponse::GetCommand() const {
  return command_;
}

string AMOResponse::GetValue() const {
  return value_;
}
}  // namespace AMOResponse
