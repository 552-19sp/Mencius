// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu

#ifndef INCLUDE_REPLICATEACK_HPP_
#define INCLUDE_REPLICATEACK_HPP_

#include <string>
#include <sstream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "AMOCommand.hpp"

namespace message {

class ReplicateAck {
 public:
  ReplicateAck();

  std::string Encode() const;

  static ReplicateAck Decode(const std::string data);

 private:
  friend class boost::serialization::access;

  template<class Archive> void serialize(Archive &ar,
      unsigned int _ /* version */) {
  }

  // TODO(ljoswiak): Include sequence number here?
  // TODO(ljoswiak): Include client address here?
};

}  // namespace message

#endif  // INCLUDE_REPLICATEACK_HPP_
