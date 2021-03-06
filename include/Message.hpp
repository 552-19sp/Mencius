// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#ifndef INCLUDE_MESSAGE_HPP_
#define INCLUDE_MESSAGE_HPP_

#include <string>
#include <sstream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "AMOCommand.hpp"

namespace message {

enum MessageType {
  kRequest,
  kResponse,
  kHeartbeat,
  kPrepare,
  kPrepareAck,
  kPropose,
  kAccept,
  kLearn,
  kDropRate
};

class Message {
 public:
  Message();
  Message(std::string encoded_message, MessageType type);

  std::string GetEncodedMessage() {
    return encoded_message_;
  }

  MessageType GetMessageType() {
    return type_;
  }

  // Serializes this Message and returns the serialized
  // output as a string.
  std::string Encode() const;

  // Deserialize the given string into a Message object.
  static Message Decode(const std::string &data);

 private:
  friend class boost::serialization::access;

  template<class Archive> void serialize(Archive &ar,
      unsigned int _ /* version */) {
    ar & encoded_message_;
    ar & type_;
  }

  std::string encoded_message_;
  MessageType type_;
};

}  // namespace message

#endif  // INCLUDE_MESSAGE_HPP_
