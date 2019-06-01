// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#include "Prepare.hpp"

namespace message {

Prepare::Prepare() {}

Prepare::Prepare(int ballot_num)
    : ballot_num_(ballot_num) {}

std::string Prepare::Encode() const {
  std::ostringstream archive_stream;
  boost::archive::text_oarchive oa(archive_stream);
  oa << *this;
  return archive_stream.str();
}

Prepare Prepare::Decode(const std::string &data) {
  Prepare m;
  std::istringstream archive_stream(data);
  boost::archive::text_iarchive archive(archive_stream);
  archive >> m;
  return m;
}

}  // namespace message
