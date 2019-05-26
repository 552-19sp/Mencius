// Copyright 2019 Jack Khuu, Justin Johnson, Lukas Joswiak

#ifndef INCLUDE_SERVERACCEPT_HPP_
#define INCLUDE_SERVERACCEPT_HPP_

#include <string>
#include <sstream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

class ServerAccept {
 public:
  ServerAccept();
  explicit ServerAccept(std::string local_port);

  std::string GetLocalPort() const {
    return local_port_;
  }

  std::string Encode() const;

  static ServerAccept Decode(const std::string data);

 private:
  friend class boost::serialization::access;

  template<class Archive> void serialize(Archive &ar,
      unsigned int _ /* version */) {
    ar & local_port_;
  }

  std::string local_port_;
};

#endif  // INCLUDE_SERVERACCEPT_HPP_
