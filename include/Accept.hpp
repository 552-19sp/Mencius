// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.
//
// Acknowledgement of a proposal.
//
// Accept(b, v)
//  - b: ballot (round) number
//  - v: the value that was accepted

#ifndef INCLUDE_ACCEPT_HPP_
#define INCLUDE_ACCEPT_HPP_

#include <string>
#include <sstream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "AMOCommand.hpp"

namespace message {

class Accept {
 public:
  Accept();
  explicit Accept(
      int ballot_num,
      KVStore::AMOCommand accepted_value);

  int GetBallotNum() const {
    return ballot_num_;
  }

  KVStore::AMOCommand GetAcceptedValue() const {
    return accepted_value_;
  }

  std::string Encode() const;

  static Accept Decode(const std::string &data);

 private:
  friend class boost::serialization::access;

  template<class Archive> void serialize(Archive &ar,
      unsigned int _ /* version */) {
    ar & ballot_num_;
    ar & accepted_value_;
  }

  int ballot_num_;
  KVStore::AMOCommand accepted_value_;
};

}  // namespace message

#endif  // INCLUDE_ACCEPT_HPP_
