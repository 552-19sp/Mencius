// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#include "Channel.hpp"

#include <iostream>

static void PrintConnections(std::set<TCPConnection::pointer> connections) {
  std::cout << "Connections:" << std::endl;
  for (auto &c : connections) {
    std::cout << "  " << c->Socket().local_endpoint() << " / "
      << c->Socket().remote_endpoint() << " ("
      << c->GetServerName() << ")" << std::endl;
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
