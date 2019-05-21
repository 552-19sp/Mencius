// Copyright 2019 Jack Khuu, Justin Johnson, Lukas Joswiak

#ifndef INCLUDE_AMORESPONSE_HPP_
#define INCLUDE_AMORESPONSE_HPP_

#include <string>

#include "AMOCommand.hpp"

namespace AMOResponse {
using std::string;

class AMOResponse {
 public:
  AMOResponse();
  AMOResponse(const AMOCommand::AMOCommand &command, const string &value);

  bool operator< (const AMOResponse &r) const;

  AMOCommand::AMOCommand GetCommand() const;
  string GetValue() const;

 private:
  AMOCommand::AMOCommand command_;
  string value_;
};
}  // namespace AMOResponse

#endif  // INCLUDE_AMORESPONSE_HPP_
