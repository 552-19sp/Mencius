// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu

#include "ReplicateAck.hpp"

namespace message {

ReplicateAck::ReplicateAck() {}

std::string ReplicateAck::Encode() const {
  std::ostringstream archive_stream;
  boost::archive::text_oarchive oa(archive_stream);
  oa << *this;
  return archive_stream.str();
}

ReplicateAck ReplicateAck::Decode(const std::string data) {
  ReplicateAck m;
  std::istringstream archive_stream(data);
  boost::archive::text_iarchive archive(archive_stream);
  archive >> m;
  return m;
}

}  // namespace message
