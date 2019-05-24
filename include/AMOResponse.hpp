// Copyright 2019 Jack Khuu, Justin Johnson, Lukas Joswiak

#ifndef INCLUDE_AMORESPONSE_HPP_
#define INCLUDE_AMORESPONSE_HPP_

#include <string>

#include "AMOCommand.hpp"

namespace AMOResponse {

class AMOResponse {
 public:
  AMOResponse();
  AMOResponse(const AMOCommand::AMOCommand &command, const std::string &value);

  bool operator< (const AMOResponse &right) const;

  AMOCommand::AMOCommand GetCommand() const;
  std::string GetValue() const;

 private:
  AMOCommand::AMOCommand command_;
  std::string value_;
};
}  // namespace AMOResponse

#endif  // INCLUDE_AMORESPONSE_HPP_
