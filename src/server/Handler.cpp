// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#include "Handler.hpp"

#include <iostream>

#include "Message.hpp"

Handler::Handler(Channel &channel) : channel_(channel) {}

void Handler::Handle(const std::string &data,
    TCPConnection::pointer connection) {
  auto m = message::Message::Decode(data);
  auto type = m.GetMessageType();
  if (type == message::MessageType::kServerSetup) {
    auto server_accept = message::ServerAccept::Decode(m.GetEncodedMessage());
    HandleServerAccept(server_accept, connection);
  } else if (type == message::MessageType::kRequest) {
    auto request = message::Request::Decode(m.GetEncodedMessage());
    HandleRequest(request, connection);
  } else if (type == message::MessageType::kReplicate) {
    auto replicate = message::Replicate::Decode(m.GetEncodedMessage());
    HandleReplicate(replicate, connection);
  } else if (type == message::MessageType::kReplicateAck) {
    auto replicate_ack = message::ReplicateAck::Decode(m.GetEncodedMessage());
    HandleReplicateAck(replicate_ack, connection);
  }
}

void Handler::Broadcast(const std::string &data) {
  channel_.Deliver(data);
  Handle(data, nullptr);
}

void Handler::Deliver(const std::string &data,
    TCPConnection::pointer connection) {
  if (connection == nullptr) {
    Handle(data, nullptr);
  } else {
    connection->Deliver(data);
  }
}

void Handler::HandleServerAccept(const message::ServerAccept &m,
    TCPConnection::pointer connection) {
  std::cout << "Received ServerAccept" << std::endl;
  channel_.Add(connection);
}

void Handler::HandleRequest(const message::Request &m,
    TCPConnection::pointer connection) {
  // TODO(ljoswiak): Replicate before executing
  std::cout << "Received request" << std::endl;
  /*
  auto amo_response = app_->Execute(m.GetCommand());
  auto response = message::Response(amo_response).Encode();
  auto encoded = message::Message(response, message::MessageType::kResponse).Encode();
  Deliver(encoded);
  */
  auto replicate = message::Replicate(m.GetCommand());
  auto message = message::Message(replicate.Encode(),
    message::MessageType::kReplicate).Encode();
  Broadcast(message);
}

void Handler::HandleReplicate(const message::Replicate &m,
    TCPConnection::pointer connection) {
  std::cout << "Received replicate" << std::endl;
  auto ack = message::ReplicateAck().Encode();
  auto message =
    message::Message(ack, message::MessageType::kReplicateAck).Encode();

  // Send response only to the server we received
  // the replicate message from.
  Deliver(message, connection);
}

void Handler::HandleReplicateAck(const message::ReplicateAck &m,
    TCPConnection::pointer connection) {
  std::cout << "Received replicate ack" << std::endl;
}
