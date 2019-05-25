// Copyright 2019 Jack Khuu, Justin Johnson, Lukas Joswiak

#ifndef INCLUDE_AMORESPONSE_HPP_
#define INCLUDE_AMORESPONSE_HPP_

#include <string>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "AMOCommand.hpp"

namespace KVStore {

class AMOResponse {
 public:
  AMOResponse();
  AMOResponse(const AMOCommand &command, const std::string &value);

  bool operator< (const AMOResponse &right) const;

  AMOCommand GetCommand() const;
  std::string GetValue() const;

 private:
  friend class boost::serialization::access;

  template<class Archive> void serialize(Archive &ar,
      unsigned int version) {
    ar & command_;
    ar & value_;
  }

  AMOCommand command_;
  std::string value_;
};
}  // namespace KVStore

#endif  // INCLUDE_AMORESPONSE_HPP_
