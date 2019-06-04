// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu

#ifndef INCLUDE_DROPRATE_HPP_
#define INCLUDE_DROPRATE_HPP_

#include <string>
#include <sstream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "AMOCommand.hpp"

namespace message {

class DropRate {
 public:
  DropRate();
  explicit DropRate(int rate);

  int GetDropRate() const {
    return drop_rate_;
  }

  std::string Encode() const;

  static DropRate Decode(const std::string &data);

 private:
  friend class boost::serialization::access;

  template<class Archive> void serialize(Archive &ar,
      unsigned int _ /* version */) {
    ar & drop_rate_;
  }

  int drop_rate_;
};

}  // namespace message

#endif  // INCLUDE_DROPRATE_HPP_
