// Copyright 2019 Jack Khuu, Justin Johnson, Lukas Joswiak

#ifndef INCLUDE_ACTION_HPP_
#define INCLUDE_ACTION_HPP_

namespace KVStore {
enum Action {
  kPut,
  kAppend,
  kGet,
  kNoOp
};
}  // namespace KVStore

#endif  // INCLUDE_ACTION_HPP_
