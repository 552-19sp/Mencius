// Copyright 2019 Jack Khuu, Justin Johnson, Lukas Joswiak

#ifndef INCLUDE_AMORESPONSE_HPP_
#define INCLUDE_AMORESPONSE_HPP_

#include <string>

#include "AMOCommand.hpp"

namespace KVStore {

class AMOResponse {
 public:
  AMOResponse();
  AMOResponse(const AMOCommand &command, const std::string &value);

  bool operator< (const AMOResponse &right) const;

  AMOCommand GetCommand() const;
  std::string GetValue() const;

 private:
  AMOCommand command_;
  std::string value_;
};
}  // namespace KVStore

#endif  // INCLUDE_AMORESPONSE_HPP_
