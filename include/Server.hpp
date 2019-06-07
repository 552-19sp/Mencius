// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.
//
// Abstract class representing core functions a server running
// Mencius should implement.

#ifndef INCLUDE_SERVER_HPP_
#define INCLUDE_SERVER_HPP_

#include <string>

#include "AMOCommand.hpp"

class Server {
 public:
  virtual std::string GetServerName() const = 0;
  virtual int GetNumServers() const = 0;

  // Handles sending the message to all servers with open
  // connections, as well as calling local handler to
  // handle message on this server.
  virtual void Broadcast(const std::string &data) = 0;

  // Handles delivery of message to a single server. If
  // the server is this server, handles delivery by
  // calling the appropriate handler, else delivers over
  // the network.
  virtual void Deliver(const std::string &data,
      const std::string &server_name) = 0;

  // Returns the coordinator of the given Mencius instance.
  virtual std::string Owner(int instance) = 0;

  // When receiving a proposal for instance i, skip all
  // unused instances prior to i this server owns.
  virtual void OnSuggestion(int instance) = 0;
  // Called when the given server is suspected of being offline.
  virtual void OnSuspect(std::string server) = 0;
  virtual void OnLearned(int instance, KVStore::AMOCommand &value) = 0;
};

#endif  // INCLUDE_SERVER_HPP_
