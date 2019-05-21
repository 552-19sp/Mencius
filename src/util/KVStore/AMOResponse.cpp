// Copyright 2019 Jack Khuu, Justin Johnson, Lukas Joswiak

#include "AMOResponse.hpp"

#include <string>

#include "AMOCommand.hpp"

namespace AMOResponse {
using std::string;

AMOResponse::AMOResponse() {}

AMOResponse::AMOResponse(const AMOCommand::AMOCommand &command,
                     const string &value) {
  command_ = command;
  value_ = value;
}

bool AMOResponse::operator< (const AMOResponse &r) const {
  return command_ < r.command_;
}

AMOCommand::AMOCommand AMOResponse::GetCommand() const {
  return command_;
}

string AMOResponse::GetValue() const {
  return value_;
}
}  // namespace AMOResponse
