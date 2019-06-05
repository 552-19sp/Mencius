// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.
//
// Represents a single round of Coordinated Paxos.

#ifndef INCLUDE_ROUND_HPP_
#define INCLUDE_ROUND_HPP_

#include <memory>
#include <string>
#include <unordered_map>

#include "Accept.hpp"
#include "AMOCommand.hpp"
#include "Learn.hpp"
#include "Prepare.hpp"
#include "PrepareAck.hpp"
#include "Propose.hpp"
#include "TCPConnection.hpp"

class TCPServer;

class Round {
 public:
  explicit Round(TCPServer *server, int instance);

  std::shared_ptr<KVStore::AMOCommand> GetLearnedValue() const {
    return learned_;
  }

  // Coordinator proposes a value.
  void Suggest(const KVStore::AMOCommand &command);
  // Coordinator proposes no-op.
  void Skip();

  // Non-coordinator starts to propose no-op.
  void Revoke();

  void HandlePropose(const message::Propose &m,
      TCPConnection::pointer connection);
  void HandlePrepare(const message::Prepare &m,
      TCPConnection::pointer connection);
  void HandlePrepareAck(const message::PrepareAck &m,
      TCPConnection::pointer connection);
  void HandleAccept(const message::Accept &m,
      TCPConnection::pointer connection);
  void HandleLearn(const message::Learn &m,
      TCPConnection::pointer connection);

 private:
  TCPServer *server_;

  int instance_;

  // Learner state.
  std::shared_ptr<KVStore::AMOCommand> learned_;
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
