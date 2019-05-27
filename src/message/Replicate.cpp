// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu

#include "Replicate.hpp"

namespace message {

Replicate::Replicate() {}

Replicate::Replicate(KVStore::AMOCommand command)
    : command_(command) {}

std::string Replicate::Encode() const {
  std::ostringstream archive_stream;
  boost::archive::text_oarchive oa(archive_stream);
  oa << *this;
  return archive_stream.str();
}

Replicate Replicate::Decode(const std::string data) {
  Replicate m;
  std::istringstream archive_stream(data);
  boost::archive::text_iarchive archive(archive_stream);
  archive >> m;
  return m;
}

}  // namespace message
