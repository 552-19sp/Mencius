// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#include "Round.hpp"

#include <cmath>
#include <iostream>

#include "TCPServer.hpp"

Round::Round(TCPServer *server)
  : server_(server),
    prepared_ballot_(0),
    accepted_ballot_(-1) {
}

void Round::Suggest(const KVStore::AMOCommand &v) {
  std::cout << server_->GetServerName() << " suggesting command" << std::endl;
  auto propose = message::Propose(0, v).Encode();
  auto message = message::Message(propose,
      message::MessageType::kPropose).Encode();
  server_->Broadcast(message);
}

void Round::Skip() {
  auto value = KVStore::AMOCommand();
  Suggest(value);
}

void Round::Revoke() {
  // TODO(ljoswiak): Implement
}

void Round::HandlePropose(const message::Propose &m,
    TCPConnection::pointer connection) {
  std::cout << server_->GetServerName() << " received propose" << std::endl;

  if (learned_) {
    // TODO(ljoswiak): Implement
    return;
  }

  auto ballot_num = m.GetBallotNum();
  auto value = m.GetValue();

  if (ballot_num == 0 && value.GetAction() == KVStore::Action::kNoOp) {
    // Learn no-op.
    learned_ = std::make_unique<KVStore::AMOCommand>();
    // TODO(ljoswiak): Implement and call OnLearned
  } else if (prepared_ballot_ <= ballot_num && accepted_ballot_ < ballot_num) {
    // Accept the ballot.
    if (ballot_num == 0) {
      // TODO(ljoswiak): Implement and call OnSuggestion
    }

    accepted_ballot_ = ballot_num;
    accepted_value_ = value;

    auto accept = message::Accept(ballot_num, value);
    auto message = message::Message(accept.Encode(),
        message::MessageType::kAccept).Encode();
    server_->Deliver(message, connection);
  }
}

void Round::HandleAccept(const message::Accept &m,
    TCPConnection::pointer connection) {
  std::cout << server_->GetServerName() << " received accept" << std::endl;

  if (learned_) {
    // TODO(ljoswiak): Implement
    return;
  }

  auto ballot_num = m.GetBallotNum();
  auto accepted_value = m.GetAcceptedValue();
  auto sender = server_->GetServerName(connection);
  std::cout << "sender: " << sender << std::endl;

  if (ballot_num == 0) {
    // TODO(ljoswiak): Implement and call OnAcceptSuggestion
  }

  learner_history_[sender] = m;
  int quorum_size =
      ceil((static_cast<double>(server_->GetNumServers()) + 1) / 2);
  if (learner_history_.size() == quorum_size) {
    // The value is now chosen. Broadcast a Learn message.
    auto learn = message::Learn(ballot_num, accepted_value);
    auto message = message::Message(learn.Encode(),
        message::MessageType::kLearn).Encode();
    server_->Broadcast(message);
  }
}

void Round::HandleLearn(const message::Learn &m,
    TCPConnection::pointer connection) {
  std::cout << server_->GetServerName() << " received learn" << std::endl;
  learned_ = std::make_unique<KVStore::AMOCommand>(m.GetValue());
}
