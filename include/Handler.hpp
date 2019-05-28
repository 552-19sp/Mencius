// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.
//
// Handles messages. One instance of a Handler exists per server,
// with TCPConnections able to call into it to handle messages
// they have received.

#ifndef INCLUDE_HANDLER_HPP_
#define INCLUDE_HANDLER_HPP_

#include <string>

#include "Channel.hpp"
#include "Replicate.hpp"
#include "ReplicateAck.hpp"
#include "Request.hpp"
#include "ServerAccept.hpp"

class Handler {
 public:
  explicit Handler(Channel &channel);

  void Handle(const std::string &data,
    TCPConnection::pointer connection);

 private:
  // Handles sending the message to all servers with open
  // connections, as well as calling local handler to
  // handle message on this server.
  void Broadcast(const std::string &data);

  // Handles delivery of message to a single server. If
  // the server is this server, handles delivery by
  // calling the appropriate handler, else delivers over
  // the network.
  void Deliver(const std::string &data,
    TCPConnection::pointer connection);

  void HandleServerAccept(const message::ServerAccept &m,
    TCPConnection::pointer connection);
  void HandleRequest(const message::Request &m,
    TCPConnection::pointer connection);
  void HandleReplicate(const message::Replicate &m,
    TCPConnection::pointer connection);
  void HandleReplicateAck(const message::ReplicateAck &m,
    TCPConnection::pointer connection);

  Channel &channel_;
};

#endif  // INCLUDE_HANDLER_HPP_
