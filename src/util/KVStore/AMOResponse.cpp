// Copyright 2019 Jack Khuu, Justin Johnson, Lukas Joswiak

#include "AMOResponse.hpp"

#include <string>

#include "AMOCommand.hpp"

namespace KVStore {
using std::string;

AMOResponse::AMOResponse() {}

AMOResponse::AMOResponse(const AMOCommand &command,
                     const string &value):command_(command), value_(value) {}

bool AMOResponse::operator< (const AMOResponse &right) const {
  return command_ < right.command_;
}

AMOCommand AMOResponse::GetCommand() const {
  return command_;
}

string AMOResponse::GetValue() const {
  return value_;
}

std::string AMOResponse::Encode() const {
  std::ostringstream archive_stream;
  boost::archive::text_oarchive oa(archive_stream);
  oa << *this;
  // TODO(ljoswiak): fix how input is read when receiving a message
  return archive_stream.str();
}

AMOResponse AMOResponse::Decode(const std::string data) {
  AMOResponse r;
  std::istringstream archive_stream(data);
  boost::archive::text_iarchive archive(archive_stream);
  archive >> r;
  return r;
}
}  // namespace KVStore
