// Copyright 2019 Lukas Joswiak, Justin Johnson, and Jack Khuu.

#include "UDPServer.hpp"

#include <stdio.h>

#include <chrono>
#include <iostream>

#include <boost/bind.hpp>

#include "Response.hpp"
#include "Utilities.hpp"

const char kConfigFilePath[] = "config";
const int kHeartbeatTimeoutMillis = 25;
const int kHeartbeatCheckTimeoutMillis = 100;

UDPServer::UDPServer(boost::asio::io_context &io_context, int port,
    std::vector<std::tuple<std::string, std::string, std::string>> &servers)
    : io_context_(io_context),
      socket_(io_context, udp::endpoint(udp::v4(), port)),
      heartbeat_timer_(io_context),
      heartbeat_check_timer_(io_context),
      app_(new KVStore::AMOStore()),
      status_(Status::kOnline),
      drop_rate_(2),
      expected_(0) {
  int counter = 0;
  for (auto tuple : servers) {
    auto server_host = std::get<0>(tuple);
    auto server_port = std::get<1>(tuple);
    auto server_name = std::get<2>(tuple);

    // Assign server name based on port it listens on.
    int port_int = std::stoi(server_port);
    if (port_int == port) {
      server_name_ = server_name;

      // Set starting round this server is responsible for
      index_ = counter;
    }

    servers_[server_name] = std::make_tuple(server_host, server_port);
    pings_[server_name] = 0;
    counter++;
  }

  std::cout << "This server's name is " << server_name_ << std::endl;

  generator_ = std::default_random_engine();
  distribution_ = std::uniform_int_distribution<int>(1, 100);

  heartbeat_timer_.expires_after(
      std::chrono::milliseconds(kHeartbeatTimeoutMillis));
  heartbeat_timer_.async_wait(
      boost::bind(&UDPServer::HeartbeatTimer, this));

  heartbeat_check_timer_.expires_after(std::chrono::seconds(7));
  heartbeat_check_timer_.async_wait(
      boost::bind(&UDPServer::HeartbeatCheckTimer, this));

  StartRead();
}

std::string UDPServer::GetServerName() const {
  return server_name_;
}

int UDPServer::GetNumServers() const {
  return servers_.size();
}

Status UDPServer::GetServerStatus() const {
  return status_;
}

void UDPServer::StartRead() {
  auto session = UDPSession::Create();
  socket_.async_receive_from(boost::asio::buffer(session->GetRecvBuffer()),
      session->GetRemoteEndpoint(), boost::bind(&UDPServer::HandleRead, this,
        session, boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}

void UDPServer::HandleRead(UDPSession::session session,
    const boost::system::error_code &ec,
    std::size_t bytes_transferred) {
  if (!ec) {
    auto buffer = session->GetRecvBuffer();
    auto message = std::string(&buffer[0],
        &buffer[0] + bytes_transferred);

    Handle(message, session);

    StartRead();
  } else {
    std::cerr << "Read error: " << ec.message() << std::endl;
  }
}

void UDPServer::StartWrite(const std::string &data, udp::endpoint &endpoint) {
  socket_.async_send_to(boost::asio::buffer(data), endpoint,
      boost::bind(&UDPServer::HandleWrite, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}

void UDPServer::HandleWrite(const boost::system::error_code &ec,
    std::size_t bytes_transferred) {}

void UDPServer::Handle(const std::string &data, UDPSession::session session) {
  auto m = message::Message::Decode(data);
  auto type = m.GetMessageType();
  auto encoded = m.GetEncodedMessage();

  std::string server_name = "";

  if (session) {
    // Get the server name by looking at who sent the message.
    auto remote_endpoint = session->GetRemoteEndpoint();
    auto remote_host = remote_endpoint.address().to_string();
    auto remote_port = std::to_string(remote_endpoint.port());
    auto remote = remote_host + ":" + remote_port;
    for (auto server : servers_) {
      auto host = std::get<0>(server.second);
      auto port = std::get<1>(server.second);

      if (remote.compare(host + ":" + port) == 0) {
        server_name = server.first;
        break;
      }
    }
  } else {
    server_name = server_name_;
  }

  switch (type) {
    case message::MessageType::kHeartbeat: {
      auto heartbeat = message::Heartbeat::Decode(encoded);
      HandleHeartbeat(heartbeat);
      break;
    }
    case message::MessageType::kRequest: {
      auto request = message::Request::Decode(encoded);
      HandleRequest(request, session);
      break;
    }
    case message::MessageType::kPrepare: {
      auto prepare = message::Prepare::Decode(encoded);
      HandlePrepare(prepare, server_name);
      break;
    }
    case message::MessageType::kPrepareAck: {
      auto prepare_ack = message::PrepareAck::Decode(encoded);
      HandlePrepareAck(prepare_ack, server_name);
      break;
    }
    case message::MessageType::kPropose: {
      auto propose = message::Propose::Decode(encoded);
      HandlePropose(propose, server_name);
      break;
    }
    case message::MessageType::kAccept: {
      auto accept = message::Accept::Decode(encoded);
      HandleAccept(accept, server_name);
      break;
    }
    case message::MessageType::kLearn: {
      auto learn = message::Learn::Decode(encoded);
      HandleLearn(learn, server_name);
      break;
    }
    default: {
      throw std::logic_error("unrecognized message type");
    }
  }
}

bool UDPServer::DropMessage() {
  int random = distribution_(generator_);
  return random <= drop_rate_;
}

void UDPServer::Broadcast(const std::string &data) {
  if (status_ == Status::kOnline) {
    for (auto server : servers_) {
      if (!DropMessage()) {
        auto server_name = server.first;
        if (server_name.compare(server_name_) == 0) {
          Handle(data, nullptr);
        } else {
          Deliver(data, server_name);
        }
      }
    }
  }
}

void UDPServer::Deliver(const std::string &data,
    const std::string &server_name) {
  if (!DropMessage()) {
    auto tuple = servers_[server_name];
    auto host = std::get<0>(tuple);
    auto port = std::get<1>(tuple);

    auto session = UDPSession::Create(io_context_, host, port);
    StartWrite(data, session->GetRemoteEndpoint());
  }
}

void UDPServer::Deliver(const std::string &data,
    UDPSession::session session) {
  if (status_ == Status::kOnline) {
    if (!DropMessage()) {
      if (!session) {
        Handle(data, nullptr);
      } else {
        StartWrite(data, session->GetRemoteEndpoint());
      }
    }
  }
}

void UDPServer::HandleHeartbeat(const message::Heartbeat &m) {
  auto server_name = m.GetServerName();
  pings_[server_name] = 0;
}

void UDPServer::HandleRequest(const message::Request &m,
    UDPSession::session session) {
  std::cout << "Received request, index = " << index_ << std::endl;

  int instance = 0;
  bool proposed = false;
  auto command = m.GetCommand();
  for (auto &kv : proposed_) {
    if (kv.second == command) {
      proposed = true;
      instance = kv.first;
      break;
    }
  }

  int seq_num = command.GetSeqNum();
  std::cout << "  seq num = " << seq_num << std::endl;
  std::cout << "  proposed = " << proposed << std::endl;
  std::cout << "  instance = " << instance << std::endl;
  std::cout << "  index_ = " << index_ << std::endl;
  std::cout << "  expected_ = " << expected_ << std::endl;
  if (!proposed) {
    auto round = std::make_shared<Round>(this, index_);

    clients_[index_] = session;
    rounds_[index_] = round;
    proposed_[index_] = command;

    std::cout << "Suggesting command for instance " << index_ << std::endl;
    round->Suggest(command);

    index_ += servers_.size();
  } else {
    // TODO(ljoswiak): Repropose
    std::cout << "REPROPOSE **********" << std::endl;
    if (instance < expected_) {
      std::cout << "  already executed, reexecuting and responding"
          << std::endl;
      // Already executed the request, re-execute and resend the response.
      auto learned = rounds_[instance]->GetLearnedValue();
      auto amo_response = app_->Execute(*learned);
      auto response = message::Response(amo_response).Encode();
      auto encoded = message::Message(response,
          message::MessageType::kResponse).Encode();
      Deliver(encoded, session);
    } else {
      std::cout << "  reproposing" << std::endl;

      if (expected_ < instance) {
        std::cout << "  retry OnSuggestion, expected_ = " << expected_
            << ", index_ = " << index_ << std::endl;
        auto round = GetRound(expected_);
        round->Skip();
      }

      // Have not learned a response yet, repropose request.
      auto round = GetRound(instance);
      round->Suggest(command);
    }
  }
}

void UDPServer::HandlePrepare(const message::Prepare &m,
    const std::string &server_name) {
  auto round = GetRound(m.GetInstance());
  round->HandlePrepare(m, server_name);
}

void UDPServer::HandlePrepareAck(const message::PrepareAck &m,
    const std::string &server_name) {
  auto round = GetRound(m.GetInstance());
  round->HandlePrepareAck(m, server_name);
}

void UDPServer::HandlePropose(const message::Propose &m,
    const std::string &server_name) {
  auto round = GetRound(m.GetInstance());
  round->HandlePropose(m, server_name);
}

void UDPServer::HandleAccept(const message::Accept &m,
    const std::string &server_name) {
  auto round = GetRound(m.GetInstance());
  round->HandleAccept(m, server_name);
}

void UDPServer::HandleLearn(const message::Learn &m,
    const std::string &server_name) {
  auto round = GetRound(m.GetInstance());
  round->HandleLearn(m, server_name);
}

void UDPServer::HandleDropRate(const message::DropRate &m) {
  drop_rate_ = m.GetDropRate();
}

std::string UDPServer::Owner(int instance) {
  auto server_addresses = Utilities::ReadConfig(kConfigFilePath);
  int index = instance % servers_.size();
  return std::get<2>(server_addresses[index]);
}

// TODO(ljoswiak): Move to Server parent class
std::shared_ptr<Round> UDPServer::GetRound(int instance) {
  if (!rounds_[instance]) {
    rounds_[instance] = std::make_shared<Round>(this, instance);
  }
  return rounds_[instance];
}

// TODO(ljoswiak): Move to Server parent class
std::shared_ptr<KVStore::AMOCommand> UDPServer::Learned(int instance) {
  return GetRound(instance)->GetLearnedValue();
}

// TODO(ljoswiak): Move to Server parent class
void UDPServer::OnSuggestion(int instance) {
  std::cout << "OnSuggestion, instance = " << instance << std::endl;

  for (int i = index_; i < instance; i += servers_.size()) {
    std::cout << "  sending skip for instance " << i << std::endl;
    auto round = GetRound(i);
    round->Skip();

    index_ += servers_.size();
  }
}

// TODO(ljoswiak): Move to Server parent class
void UDPServer::OnSuspect(const std::string &server) {
  for (int i = expected_; i < index_; i++) {
    auto owner = Owner(i);
    if (owner.compare(server) == 0 && !Learned(i)) {
      std::cout << "  sending revoke for instance " << i << std::endl;
      auto round = GetRound(i);
      round->Revoke();
    }
  }
}

void UDPServer::OnLearned(int instance, const KVStore::AMOCommand &value) {
  std::cout << "OnLearned, instance = " << instance << std::endl;

  auto instance_owner = Owner(instance);
  auto proposed_command = proposed_[instance];
  if (instance_owner.compare(server_name_) != 0 &&
      proposed_[instance] == value) {
    // TODO(ljoswiak): Propose value in proposed_[instance]
  }

  CheckCommit();
}

// TODO(ljoswiak): Move shared code to Server parent class
void UDPServer::CheckCommit() {
  std::cout << "CheckCommit. expected = " << expected_ << std::endl;
  while (rounds_.find(expected_) != rounds_.end()) {
    std::cout << "  expected: " << expected_ << std::endl;
    auto learned = rounds_[expected_]->GetLearnedValue();
    if (!learned) {
      std::cout << "  haven't learned yet" << std::endl;
      break;
    }

    if (learned->GetAction() != KVStore::Action::kNoOp) {
      // Value is committed. Execute and return to client.
      std::cout << "  committing value for instance " << expected_ << std::endl;

      if (learned->GetAction() == KVStore::Action::kKillServer
          && status_ == Status::kOnline) {
        auto server = learned->GetKey();
        std::cout << "  killing server " << server << std::endl;
        if (server.compare(server_name_) == 0) {
          // If I as a server have been killed, I should continue
          // to handle messages, but should drop all outgoing
          // messages.
          status_ = Status::kOffline;
        } else {
          OnSuspect(server);
        }
      } else if (learned->GetAction() == KVStore::Action::kReviveServer
          && status_ == Status::kOffline) {
        auto server = learned->GetKey();
        if (server.compare(server_name_) == 0) {
          status_ = Status::kOnline;
        }
      }

      auto amo_response = app_->Execute(*learned);
      executed_[expected_] = amo_response;

      auto client_session = clients_[expected_];
      if  (client_session) {
        // If client connection is null, this value is being
        // learned on a server that did not receive the request,
        // so no need to send a reply.
        auto response = message::Response(amo_response).Encode();
        auto encoded = message::Message(response,
            message::MessageType::kResponse).Encode();
        Deliver(encoded, client_session);
        clients_.erase(expected_);
      }
    } else {
      std::cout << "  committing no-op for instance " << expected_ << std::endl;
    }

    expected_++;
  }
  std::cout << "  end of CheckCommit. expected = " << expected_ << std::endl;
}

void UDPServer::HeartbeatTimer() {
  auto heartbeat = message::Heartbeat(server_name_).Encode();
  auto message = message::Message(heartbeat,
      message::MessageType::kHeartbeat).Encode();
  Broadcast(message);

  heartbeat_timer_.expires_at(heartbeat_timer_.expiry() +
      std::chrono::milliseconds(kHeartbeatTimeoutMillis));
  heartbeat_timer_.async_wait(
      boost::bind(&UDPServer::HeartbeatTimer, this));
}

void UDPServer::HeartbeatCheckTimer() {
  for (auto &kv : pings_) {
    kv.second++;

    if (kv.second > 1) {
      OnSuspect(kv.first);
    }
  }

  heartbeat_check_timer_.expires_at(heartbeat_check_timer_.expiry() +
      std::chrono::milliseconds(kHeartbeatCheckTimeoutMillis));
  heartbeat_check_timer_.async_wait(
      boost::bind(&UDPServer::HeartbeatCheckTimer, this));
}

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
    return 0;
  }

  int port = atoi(argv[1]);

  auto server_addresses = Utilities::ReadConfig(kConfigFilePath);

  boost::asio::io_context io_context;
  UDPServer server(io_context, port, server_addresses);
  io_context.run();

  return 1;
}
