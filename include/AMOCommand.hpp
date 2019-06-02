// Copyright 2019 Jack Khuu, Justin Johnson, Lukas Joswiak

#ifndef INCLUDE_AMOCOMMAND_HPP_
#define INCLUDE_AMOCOMMAND_HPP_

#include <string>
#include <sstream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "Action.hpp"

namespace KVStore {

class AMOCommand {
 public:
  // Create a no-op command.
  AMOCommand();
  AMOCommand(int seq_num, std::string key, std::string value, Action action);
  AMOCommand(const AMOCommand &command);
  ~AMOCommand();

  bool operator< (const AMOCommand &r) const;

  int GetSeqNum() const;
  std::string GetKey() const;
  std::string GetValue() const;
  Action GetAction() const;

 private:
  friend class boost::serialization::access;

  template<class Archive> void serialize(Archive &ar,
      unsigned int _ /* version */) {
    ar & seq_num_;
    ar & key_;
    ar & value_;
    ar & action_;
  }

  int seq_num_;
  std::string key_;
  std::string value_;
  Action action_;
};
}  // namespace KVStore

#endif  // INCLUDE_AMOCOMMAND_HPP_
