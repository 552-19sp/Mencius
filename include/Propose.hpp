// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.
//
// Use to propose a value in the round.
//
// Propose(b, v)
//  - b: ballot (round) number
//  - v: the value to propose

#ifndef INCLUDE_PROPOSE_HPP_
#define INCLUDE_PROPOSE_HPP_

#include <string>
#include <sstream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "AMOCommand.hpp"

namespace message {

class Propose {
 public:
  Propose();
  explicit Propose(
      int ballot_num,
      KVStore::AMOCommand value);

  int GetBallotNum() const {
    return ballot_num_;
  }

  KVStore::AMOCommand GetValue() const {
    return value_;
  }

  std::string Encode() const;

  static Propose Decode(const std::string &data);

 private:
  friend class boost::serialization::access;

  template<class Archive> void serialize(Archive &ar,
      unsigned int _ /* version */) {
    ar & ballot_num_;
    ar & value_;
  }

  int ballot_num_;
  KVStore::AMOCommand value_;
};

}  // namespace message

#endif  // INCLUDE_PROPOSE_HPP_
