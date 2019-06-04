// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu

#include "DropRate.hpp"

namespace message {

DropRate::DropRate()
    : drop_rate_(0) {}

DropRate::DropRate(int drop_rate)
    : drop_rate_(drop_rate) {}

std::string DropRate::Encode() const {
  std::ostringstream archive_stream;
  boost::archive::text_oarchive oa(archive_stream);
  oa << *this;
  return archive_stream.str();
}

DropRate DropRate::Decode(const std::string &data) {
  DropRate m;
  std::istringstream archive_stream(data);
  boost::archive::text_iarchive archive(archive_stream);
  archive >> m;
  return m;
}

}  // namespace message
