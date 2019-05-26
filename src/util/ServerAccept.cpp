// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu"

#include "ServerAccept.hpp"

ServerAccept::ServerAccept() {}

ServerAccept::ServerAccept(std::string local_port)
    : local_port_(local_port) {}

std::string ServerAccept::Encode() const {
  std::ostringstream archive_stream;
  boost::archive::text_oarchive oa(archive_stream);
  oa << *this;
  return archive_stream.str();
}

ServerAccept ServerAccept::Decode(const std::string data) {
  ServerAccept m;
  std::istringstream archive_stream(data);
  boost::archive::text_iarchive archive(archive_stream);
  archive >> m;
  return m;
}
