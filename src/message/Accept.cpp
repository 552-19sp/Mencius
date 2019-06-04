// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#include "Accept.hpp"

namespace message {

Accept::Accept() {}

Accept::Accept(
    int instance,
    int ballot_num,
    KVStore::AMOCommand accepted_value)
    : instance_(instance),
      ballot_num_(ballot_num),
      accepted_value_(accepted_value) {}

std::string Accept::Encode() const {
  std::ostringstream archive_stream;
  boost::archive::text_oarchive oa(archive_stream);
  oa << *this;
  return archive_stream.str();
}

Accept Accept::Decode(const std::string &data) {
  Accept m;
  std::istringstream archive_stream(data);
  boost::archive::text_iarchive archive(archive_stream);
  archive >> m;
  return m;
}

}  // namespace message
