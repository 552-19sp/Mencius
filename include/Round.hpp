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
#include "Message.hpp"
#include "Prepare.hpp"
#include "PrepareAck.hpp"
#include "Propose.hpp"
#include "Server.hpp"

class Round {
 public:
  explicit Round(Server *server, int instance);

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
      const std::string &server_name);
  void HandlePrepare(const message::Prepare &m,
      const std::string &server_name);
  void HandlePrepareAck(const message::PrepareAck &m,
      const std::string &server_name);
  void HandleAccept(const message::Accept &m,
      const std::string &server_name);
  void HandleLearn(const message::Learn &m,
      const std::string &server_name);

 private:
  void AlreadyLearned(const std::string &server_name);

  Server *server_;

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
