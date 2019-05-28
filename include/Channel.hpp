// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#ifndef INCLUDE_CHANNEL_HPP_
#define INCLUDE_CHANNEL_HPP_

#include <set>
#include <string>

#include "TCPConnection.hpp"

class Channel {
 public:
  void Add(TCPConnection::pointer connection);
  void Remove(TCPConnection::pointer connection);
  // Deliver message to all connections in the channel.
  void Deliver(const std::string &message);
 private:
  std::set<TCPConnection::pointer> connections_;
};

#endif  // INCLUDE_CHANNEL_HPP_
