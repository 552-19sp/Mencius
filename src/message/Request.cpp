// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu

#include "Request.hpp"

namespace message {

Request::Request() {}

Request::Request(KVStore::AMOCommand command)
    : command_(command) {}

std::string Request::Encode() const {
  std::ostringstream archive_stream;
  boost::archive::text_oarchive oa(archive_stream);
  oa << *this;
  return archive_stream.str();
}

Request Request::Decode(const std::string &data) {
  Request m;
  std::istringstream archive_stream(data);
  boost::archive::text_iarchive archive(archive_stream);
  archive >> m;
  return m;
}

}  // namespace message
