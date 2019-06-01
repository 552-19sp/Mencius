// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu

#ifndef INCLUDE_REQUEST_HPP_
#define INCLUDE_REQUEST_HPP_

#include <string>
#include <sstream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "AMOCommand.hpp"

namespace message {

class Request {
 public:
  Request();
  explicit Request(KVStore::AMOCommand command);

  KVStore::AMOCommand GetCommand() const {
    return command_;
  }

  std::string Encode() const;

  static Request Decode(const std::string &data);

 private:
  friend class boost::serialization::access;

  template<class Archive> void serialize(Archive &ar,
      unsigned int _ /* version */) {
    ar & command_;
  }

  KVStore::AMOCommand command_;
};

}  // namespace message

#endif  // INCLUDE_REQUEST_HPP_
