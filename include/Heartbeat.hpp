// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#ifndef INCLUDE_HEARTBEAT_HPP_
#define INCLUDE_HEARTBEAT_HPP_

#include <string>
#include <sstream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

namespace message {

class Heartbeat {
 public:
  Heartbeat();
  explicit Heartbeat(std::string server_name);

  std::string GetServerName() const {
    return server_name_;
  }

  std::string Encode() const;

  static Heartbeat Decode(const std::string &data);

 private:
  friend class boost::serialization::access;

  template<class Archive> void serialize(Archive &ar,
      unsigned int _ /* version */) {
    ar & server_name_;
  }

  std::string server_name_;
};

}  // namespace message

#endif  // INCLUDE_HEARTBEAT_HPP_
