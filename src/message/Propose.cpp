// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#include "Propose.hpp"

namespace message {

Propose::Propose() {}

Propose::Propose(
    int instance,
    int ballot_num,
    KVStore::AMOCommand value)
    : instance_(instance),
      ballot_num_(ballot_num),
      value_(value) {}

std::string Propose::Encode() const {
  std::ostringstream archive_stream;
  boost::archive::text_oarchive oa(archive_stream);
  oa << *this;
  return archive_stream.str();
}

Propose Propose::Decode(const std::string &data) {
  Propose m;
  std::istringstream archive_stream(data);
  boost::archive::text_iarchive archive(archive_stream);
  archive >> m;
  return m;
}

}  // namespace message
