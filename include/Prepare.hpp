// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#ifndef INCLUDE_PREPARE_HPP_
#define INCLUDE_PREPARE_HPP_

#include <string>
#include <sstream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

namespace message {

class Prepare {
 public:
  Prepare();
  explicit Prepare(int ballot_num);

  int GetBallotNum() const {
    return ballot_num_;
  }

  std::string Encode() const;

  static Prepare Decode(const std::string &data);

 private:
  friend class boost::serialization::access;

  template<class Archive> void serialize(Archive &ar,
      unsigned int _ /* version */) {
    ar & ballot_num_;
  }

  int ballot_num_;
};

}  // namespace message

#endif  // INCLUDE_PREPARE_HPP_
