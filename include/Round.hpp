// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.
//
// Represents a single round of Coordinated Paxos.

#ifndef INCLUDE_ROUND_HPP_
#define INCLUDE_ROUND_HPP_

#include <string>
#include <unordered_map>

#include "Accept.hpp"
#include "AMOCommand.hpp"
#include "TCPConnection.hpp"
#include "TCPServer.hpp"

class Round {
 public:
  Round();

  // Coordinator proposes a value.
  void Suggest(const KVStore::AMOCommand v);
  // Coordinator proposes no-op.
  void Skip();

  // Non-coordinator starts to propose no-op.
  void Revoke();

 private:
  TCPServer *server_;

  // Learner state.
  KVStore::AMOCommand learned_;
  std::unordered_map<std::string, message::Accept> learner_history_;

  // Proposer state.
  std::unordered_map<std::string, message::PrepareAck>
    prepared_history_;

  // Acceptor state.
  int prepared_ballot_;
  int accepted_ballot_;
  KVStore::AMOCommand accepted_value_;
};

#endif  // INCLUDE_ROUND_HPP_
