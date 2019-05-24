// Copyright 2019 Jack Khuu, Justin Johnson, Lukas Joswiak

#ifndef INCLUDE_AMOSTORE_HPP_
#define INCLUDE_AMOSTORE_HPP_

#include <map>
#include <string>

#include "AMOCommand.hpp"
#include "AMOResponse.hpp"
#include "KVStore.hpp"

namespace AMOStore {

class AMOStore {
 public:
  AMOStore();

  AMOResponse::AMOResponse Execute(const AMOCommand::AMOCommand &command);

 private:
  KVStore::KVStore kvStore_;
  std::map<AMOCommand::AMOCommand, AMOResponse::AMOResponse> prev_;

  bool AlreadyExecuted(const AMOCommand::AMOCommand &command) const;
};

}   // namespace AMOStore

#endif  // INCLUDE_AMOSTORE_HPP_
