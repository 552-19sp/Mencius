// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#ifndef INCLUDE_CHANNEL_HPP_
#define INCLUDE_CHANNEL_HPP_

#include <set>
#include <string>
#include <tuple>
#include <vector>

#include "TCPConnection.hpp"

class Channel {
 public:
  void Add(TCPConnection::pointer connection);
  void Remove(TCPConnection::pointer connection);
  // Deliver message to all connections in the channel.
  void Deliver(const std::string &message);

  // Given a server name, return the associated TCPConnection
  // object, or nullptr if it doesn't exist.
  TCPConnection::pointer GetConnection(std::string server_name);

  // Returns a list of servers that don't currently have open
  // connections, based on the overall server list passed as
  // a parameter.
  std::vector<std::string> OfflineServers(
      std::vector<std::tuple<std::string, std::string, std::string>> &servers,
      std::string &self_name);

 private:
  std::set<TCPConnection::pointer> connections_;
};

#endif  // INCLUDE_CHANNEL_HPP_
