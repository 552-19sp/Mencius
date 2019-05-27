// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu

#include "Response.hpp"

namespace message {

Response::Response() {}

Response::Response(KVStore::AMOResponse response)
    : response_(response) {}

std::string Response::Encode() const {
  std::ostringstream archive_stream;
  boost::archive::text_oarchive oa(archive_stream);
  oa << *this;
  return archive_stream.str();
}

Response Response::Decode(const std::string data) {
  Response m;
  std::istringstream archive_stream(data);
  boost::archive::text_iarchive archive(archive_stream);
  archive >> m;
  return m;
}

}  // namespace message
