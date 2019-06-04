// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu

#include "KillServer.hpp"

namespace message {

KillServer::KillServer() {}

std::string KillServer::Encode() const {
  std::ostringstream archive_stream;
  boost::archive::text_oarchive oa(archive_stream);
  oa << *this;
  return archive_stream.str();
}

KillServer KillServer::Decode(const std::string &data) {
  KillServer m;
  std::istringstream archive_stream(data);
  boost::archive::text_iarchive archive(archive_stream);
  archive >> m;
  return m;
}

}  // namespace message
