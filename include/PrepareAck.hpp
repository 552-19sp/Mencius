// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.
//
// Acknowledgement to a Prepare message.
//
// PrepareAck(b, ab, av)
//  - b: ballot (round) number
//  - ab: highest ballot this server has accepted a value in
//  - av: the value accepted for round ab

#ifndef INCLUDE_PREPAREACK_HPP_
#define INCLUDE_PREPAREACK_HPP_

#include <string>
#include <sstream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "AMOCommand.hpp"

namespace message {

class PrepareAck {
 public:
  PrepareAck();
  explicit PrepareAck(
      int ballot_num,
      int accepted_ballot,
      KVStore::AMOCommand accepted_value);

  int GetBallotNum() const {
    return ballot_num_;
  }

  int GetAcceptedBallot() const {
    return accepted_ballot_;
  }

  KVStore::AMOCommand GetAcceptedValue() const {
    return accepted_value_;
  }

  std::string Encode() const;

  static PrepareAck Decode(const std::string &data);

 private:
  friend class boost::serialization::access;

  template<class Archive> void serialize(Archive &ar,
      unsigned int _ /* version */) {
    ar & ballot_num_;
    ar & accepted_ballot_;
    ar & accepted_value_;
  }

  int ballot_num_;
  int accepted_ballot_;
  KVStore::AMOCommand accepted_value_;
};

}  // namespace message

#endif  // INCLUDE_PREPAREACK_HPP_
