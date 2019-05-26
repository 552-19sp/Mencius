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
  AMOCommand() {}
  AMOCommand(int seq_num, const std::string &key, const std::string &value,
                   const Action &action);

  bool operator< (const AMOCommand &r) const;

  int GetSeqNum() const;
  std::string GetKey() const;
  std::string GetValue() const;
  Action GetAction() const;

  std::string Encode() const;

  static AMOCommand Decode(const std::string data);

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
