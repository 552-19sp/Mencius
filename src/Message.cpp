// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu"

#include "Message.hpp"

Message::Message() {}

Message::Message(std::string m, MessageType type) : m_(m), type_(type) {}

std::string Message::Encode() const {
  std::ostringstream archive_stream;
  boost::archive::text_oarchive oa(archive_stream);
  oa << *this;
  // TODO(ljoswiak): fix how input is read when receiving a message
  return archive_stream.str() + "\n";
}

Message Message::Decode(const std::string data) {
  Message m;
  std::istringstream archive_stream(data);
  boost::archive::text_iarchive archive(archive_stream);
  archive >> m;
  return m;
}
