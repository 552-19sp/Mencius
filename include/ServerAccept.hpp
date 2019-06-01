// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu

#ifndef INCLUDE_SERVERACCEPT_HPP_
#define INCLUDE_SERVERACCEPT_HPP_

#include <string>
#include <sstream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

namespace message {

class ServerAccept {
 public:
  ServerAccept();
  explicit ServerAccept(std::string server_name);

  std::string GetServerName() const {
    return server_name_;
  }

  std::string Encode() const;

  static ServerAccept Decode(const std::string &data);

 private:
  friend class boost::serialization::access;

  template<class Archive> void serialize(Archive &ar,
      unsigned int _ /* version */) {
    ar & server_name_;
  }

  std::string server_name_;
};

}  // namespace message

#endif  // INCLUDE_SERVERACCEPT_HPP_
