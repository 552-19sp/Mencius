// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#include "PrepareAck.hpp"

namespace message {

PrepareAck::PrepareAck() {}

PrepareAck::PrepareAck(
    int instance,
    int ballot_num,
    int accepted_ballot,
    KVStore::AMOCommand accepted_value)
    : instance_(instance),
      ballot_num_(ballot_num),
      accepted_ballot_(accepted_ballot),
      accepted_value_(accepted_value) {}

std::string PrepareAck::Encode() const {
  std::ostringstream archive_stream;
  boost::archive::text_oarchive oa(archive_stream);
  oa << *this;
  return archive_stream.str();
}

PrepareAck PrepareAck::Decode(const std::string &data) {
  PrepareAck m;
  std::istringstream archive_stream(data);
  boost::archive::text_iarchive archive(archive_stream);
  archive >> m;
  return m;
}

}  // namespace message
