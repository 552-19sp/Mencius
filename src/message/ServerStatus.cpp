// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#include "ServerStatus.hpp"

namespace message {

ServerStatus::ServerStatus() {}

ServerStatus::ServerStatus(std::string server_name, Status status)
    : server_name_(server_name),
      status_(status) {}

std::string ServerStatus::Encode() const {
  std::ostringstream archive_stream;
  boost::archive::text_oarchive oa(archive_stream);
  oa << *this;
  return archive_stream.str();
}

ServerStatus ServerStatus::Decode(const std::string &data) {
  ServerStatus m;
  std::istringstream archive_stream(data);
  boost::archive::text_iarchive archive(archive_stream);
  archive >> m;
  return m;
}

}  // namespace message
