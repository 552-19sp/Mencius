// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#include "Channel.hpp"

#include <iostream>

static void PrintConnections(std::set<TCPConnection::pointer> connections) {
  std::cout << "Connections:" << std::endl;
  for (auto &c : connections) {
    std::cout << "  " << c->Socket().local_endpoint() << " / "
      << c->Socket().remote_endpoint() << " ("
      << c->GetServerName() << ") (status: " << c->GetServerStatus()
      << ")" << std::endl;
  }
}

void Channel::Add(TCPConnection::pointer connection) {
  connections_.insert(connection);

  PrintConnections(connections_);
}

void Channel::Remove(TCPConnection::pointer connection) {
  std::cout << "channel remove" << std::endl;
  connections_.erase(connection);

  PrintConnections(connections_);
}

void Channel::Deliver(const std::string &message) {
  // Call each TCPConnection's deliver method
  std::cout << "Channel delivering to" << std::endl;
  PrintConnections(connections_);
  std::for_each(connections_.begin(), connections_.end(),
    boost::bind(&TCPConnection::Deliver, _1, boost::ref(message)));
}

std::vector<std::string> Channel::OfflineServers(
    std::vector<std::tuple<std::string, std::string, std::string>> &servers,
    std::string &self_name) {
  std::vector<std::string> online_servers;
  for (auto connection : connections_) {
    auto server_name = connection->GetServerName();
    online_servers.push_back(server_name);
  }

  std::vector<std::string> offline_servers;
  for (auto tuple : servers) {
    auto server_name = std::get<2>(tuple);
    if (std::find(online_servers.begin(), online_servers.end(), server_name) ==
        online_servers.end() && self_name.compare(server_name) != 0) {
      // Server is offline.
      offline_servers.push_back(server_name);
    }
  }

  return offline_servers;
}
