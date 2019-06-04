// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#include "Learn.hpp"

namespace message {

Learn::Learn() {}

Learn::Learn(
    int instance,
    KVStore::AMOCommand value)
    : instance_(instance),
      value_(value) {}

std::string Learn::Encode() const {
  std::ostringstream archive_stream;
  boost::archive::text_oarchive oa(archive_stream);
  oa << *this;
  return archive_stream.str();
}

Learn Learn::Decode(const std::string &data) {
  Learn m;
  std::istringstream archive_stream(data);
  boost::archive::text_iarchive archive(archive_stream);
  archive >> m;
  return m;
}

}  // namespace message
