// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.
//
// Represents a single round of Coordinated Paxos.

#ifndef INCLUDE_ROUND_HPP_
#define INCLUDE_ROUND_HPP_

/*
class Round {
 public:
  Round();

  // Coordinator proposes a value.
  // Suggest(? v);
  // Coordinator proposes no-op.
  void Skip();

  // Non-coordinator starts to propose no-op.
  void Revoke();

 private:
  // Learner state.
  // KVStore::AMOResponse learned_;
  // ? learner_history_;

  // Proposer state.
  // ? prepared_history_;

  // Acceptor state.
  // uint64_t prepared_ballot_;
  // int64_t accepted_ballot_;
  // ? accepted_value_;
}
*/

#endif  // INCLUDE_ROUND_HPP_
