// Copyright 2019 Jack Khuu, Justin Johnson, Lukas Joswiak

#ifndef INCLUDE_AMOSTORE_HPP_
#define INCLUDE_AMOSTORE_HPP_

#include <map>
#include <string>

#include "AMOCommand.hpp"
#include "AMOResponse.hpp"
#include "KVStore.hpp"

namespace KVStore {

class AMOStore {
 public:
  AMOStore();

  AMOResponse Execute(const AMOCommand &command);

 private:
  KVStore kvStore_;
  std::map<AMOCommand, AMOResponse> prev_;

  bool AlreadyExecuted(const AMOCommand &command) const;
};

}   // namespace KVStore

#endif  // INCLUDE_AMOSTORE_HPP_
