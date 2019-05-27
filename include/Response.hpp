// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu

#ifndef INCLUDE_RESPONSE_HPP_
#define INCLUDE_RESPONSE_HPP_

#include <string>
#include <sstream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "AMOResponse.hpp"

namespace message {

class Response {
 public:
  Response();
  explicit Response(KVStore::AMOResponse response);

  KVStore::AMOResponse GetResponse() const {
    return response_;
  }

  std::string Encode() const;

  static Response Decode(const std::string data);

 private:
  friend class boost::serialization::access;

  template<class Archive> void serialize(Archive &ar,
      unsigned int _ /* version */) {
    ar & response_;
  }

  KVStore::AMOResponse response_;
};

}  // namespace message

#endif  // INCLUDE_RESPONSE_HPP_
