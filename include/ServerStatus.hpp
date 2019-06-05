// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.
//
// Use this message to toggle the online status of a server.

#ifndef INCLUDE_SERVERSTATUS_HPP_
#define INCLUDE_SERVERSTATUS_HPP_

#include <string>
#include <sstream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

namespace message {

enum Status {
  kOffline,
  kOnline
};

class ServerStatus {
 public:
  ServerStatus();
  ServerStatus(std::string server_name, Status status);

  std::string GetServerName() const {
    return server_name_;
  }

  Status GetServerStatus() const {
    return status_;
  }

  std::string Encode() const;

  static ServerStatus Decode(const std::string &data);

 private:
  friend class boost::serialization::access;

  template<class Archive> void serialize(Archive &ar,
      unsigned int _ /* version */) {
    ar & server_name_;
  }

  std::string server_name_;
  Status status_;
};

}  // namespace message

#endif  // INCLUDE_SERVERSTATUS_HPP_
