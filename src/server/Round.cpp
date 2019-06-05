// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#include "Round.hpp"

#include <cmath>
#include <iostream>

#include "TCPServer.hpp"

Round::Round(TCPServer *server, int instance)
  : server_(server),
    instance_(instance),
    prepared_ballot_(0),
    accepted_ballot_(-1) {
}

int QuorumSize(int num_servers) {
  return ceil((static_cast<double>(num_servers) + 1) / 2);
}

void Round::Suggest(const KVStore::AMOCommand &v) {
  std::cout << server_->GetServerName() << " suggesting command for instance "
      << instance_ << std::endl;
  auto propose = message::Propose(instance_, 0, v).Encode();
  auto message = message::Message(propose,
      message::MessageType::kPropose).Encode();
  server_->Broadcast(message);
}

void Round::Skip() {
  auto value = KVStore::AMOCommand();
  Suggest(value);
}

void Round::Revoke() {
  int ballot_num = 0;
  while (server_->Owner(ballot_num).compare(server_->GetServerName()) != 0 ||
      ballot_num <= prepared_ballot_ || ballot_num <= accepted_ballot_) {
    ballot_num++;
  }

  std::cout << "Revoking with ballot: " << ballot_num << std::endl;

  auto prepare = message::Prepare(instance_, ballot_num).Encode();
  auto message = message::Message(prepare,
      message::MessageType::kPrepare).Encode();
  server_->Broadcast(message);
}

void Round::HandlePropose(const message::Propose &m,
    TCPConnection::pointer connection) {
  std::cout << server_->GetServerName() << " received propose in instance "
      << instance_ << std::endl;

  if (learned_) {
    // TODO(ljoswiak): Implement
    std::cout << "  already learned a value, returning" << std::endl;
    return;
  }

  auto ballot_num = m.GetBallotNum();
  auto value = m.GetValue();

  if (ballot_num == 0 && value.GetAction() == KVStore::Action::kNoOp) {
    // Learn no-op.
    std::cout << "  learning no-op" << std::endl;
    learned_ = std::make_shared<KVStore::AMOCommand>();
    server_->OnLearned(instance_, *learned_);
  } else if (prepared_ballot_ <= ballot_num && accepted_ballot_ < ballot_num) {
    // Accept the ballot.
    if (ballot_num == 0) {
      server_->OnSuggestion(instance_);
    }

    accepted_ballot_ = ballot_num;
    accepted_value_ = value;

    auto accept = message::Accept(instance_, ballot_num, value);
    auto message = message::Message(accept.Encode(),
        message::MessageType::kAccept).Encode();
    server_->Deliver(message, connection);
  }
}

void Round::HandlePrepare(const message::Prepare &m,
    TCPConnection::pointer connection) {
  if (learned_) {
    // TODO(ljoswiak): Implement
    return;
  }

  auto ballot_num = m.GetBallotNum();
  if (ballot_num > prepared_ballot_) {
    prepared_ballot_ = ballot_num;
    auto prepare_ack = message::PrepareAck(instance_, ballot_num,
        accepted_ballot_, accepted_value_).Encode();
    auto message = message::Message(prepare_ack,
        message::MessageType::kPrepareAck).Encode();
    server_->Deliver(message, connection);
  }
}

void Round::HandlePrepareAck(const message::PrepareAck &m,
    TCPConnection::pointer connection) {
  if (learned_) {
    // TODO(ljoswiak): Implement
    return;
  }

  int ballot_num = m.GetBallotNum();
  auto sender = server_->GetServerName(connection);

  prepared_history_[sender] = m;
  int quorum_size = QuorumSize(server_->GetNumServers());
  if (prepared_history_.size() == quorum_size) {
    // Find the highest accepted ballot and the associated
    // value, then propose it.

    int highest_accepted = -1;
    // Default command to no-op.
    KVStore::AMOCommand value = KVStore::AMOCommand();
    for (const auto &kv : prepared_history_) {
      auto kv_accepted_ballot = kv.second.GetAcceptedBallot();
      if (kv_accepted_ballot > highest_accepted) {
        highest_accepted = kv_accepted_ballot;
        value = kv.second.GetAcceptedValue();
      }
    }

    // Now propose the highest accepted value.
    std::cout << "HandlePrepareAck: Proposing command for instance "
        << instance_ << std::endl;
    auto propose = message::Propose(instance_, ballot_num, value).Encode();
    auto message = message::Message(propose,
        message::MessageType::kPropose).Encode();
    server_->Broadcast(message);
  }
}

void Round::HandleAccept(const message::Accept &m,
    TCPConnection::pointer connection) {
  std::cout << server_->GetServerName() << " received accept in instance "
      << instance_ << std::endl;

  if (learned_) {
    // TODO(ljoswiak): Implement
    return;
  }

  auto ballot_num = m.GetBallotNum();
  auto accepted_value = m.GetAcceptedValue();
  auto sender = server_->GetServerName(connection);

  if (ballot_num == 0) {
    // TODO(ljoswiak): Implement and call OnAcceptSuggestion
  }

  learner_history_[sender] = m;
  int quorum_size = QuorumSize(server_->GetNumServers());
  if (learner_history_.size() == quorum_size) {
    // The value is now chosen. Broadcast a Learn message.
    auto learn = message::Learn(instance_, accepted_value);
    auto message = message::Message(learn.Encode(),
        message::MessageType::kLearn).Encode();
    server_->Broadcast(message);
  }
}

void Round::HandleLearn(const message::Learn &m,
    TCPConnection::pointer connection) {
  std::cout << server_->GetServerName() << " received learn in instance "
      << instance_ << std::endl;
  learned_ = std::make_shared<KVStore::AMOCommand>(m.GetValue());
  server_->OnLearned(m.GetInstance(), *learned_);
  // TODO(ljoswiak): Implement and call OnLearned
}
