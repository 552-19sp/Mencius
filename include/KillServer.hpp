// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu

#ifndef INCLUDE_KILLSERVER_HPP_
#define INCLUDE_KILLSERVER_HPP_

#include <string>
#include <sstream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "AMOCommand.hpp"

namespace message {

class KillServer {
 public:
  KillServer();

  std::string Encode() const;

  static KillServer Decode(const std::string &data);

 private:
  friend class boost::serialization::access;

  template<class Archive> void serialize(Archive &ar,
      unsigned int _ /* version */) {}
};

}  // namespace message

#endif  // INCLUDE_KILLSERVER_HPP_
