// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#include "Heartbeat.hpp"

namespace message {

Heartbeat::Heartbeat() {}

Heartbeat::Heartbeat(std::string server_name)
    : server_name_(server_name) {}

std::string Heartbeat::Encode() const {
  std::ostringstream archive_stream;
  boost::archive::text_oarchive oa(archive_stream);
  oa << *this;
  return archive_stream.str();
}

Heartbeat Heartbeat::Decode(const std::string &data) {
  Heartbeat m;
  std::istringstream archive_stream(data);
  boost::archive::text_iarchive archive(archive_stream);
  archive >> m;
  return m;
}

}  // namespace message
