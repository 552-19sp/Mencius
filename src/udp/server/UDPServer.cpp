// Copyright 2019 Lukas Joswiak, Justin Johnson, and Jack Khuu.

#include "UDPServer.hpp"

#include <stdio.h>

#include <iostream>

#include <boost/bind.hpp>

#include "Response.hpp"
#include "Utilities.hpp"

const char kConfigFilePath[] = "config";

UDPServer::UDPServer(boost::asio::io_context &io_context, int port,
    std::vector<std::tuple<std::string, std::string, std::string>> &servers)
    : io_context_(io_context),
      socket_(io_context, udp::endpoint(udp::v4(), port)),
      app_(new KVStore::AMOStore()),
      status_(Status::kOnline),
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
    counter++;
  }

  std::cout << "This server's name is " << server_name_ << std::endl;

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

  std::cout << "Received message from " << server_name << std::endl;

  switch (type) {
    case message::MessageType::kRequest: {
      auto request = message::Request::Decode(encoded);
      HandleRequest(request, session);
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

void UDPServer::Broadcast(const std::string &data) {
  if (status_ == Status::kOnline) {
    for (auto server : servers_) {
      auto server_name = server.first;
      if (server_name.compare(server_name_) == 0) {
        Handle(data, nullptr);
      } else {
        Deliver(data, server_name);
      }
    }
  }
}

void UDPServer::Deliver(const std::string &data,
    const std::string &server_name) {
  auto tuple = servers_[server_name];
  auto host = std::get<0>(tuple);
  auto port = std::get<1>(tuple);

  auto session = UDPSession::Create(io_context_, host, port);
  StartWrite(data, session->GetRemoteEndpoint());
}

void UDPServer::Deliver(const std::string &data,
    UDPSession::session session) {
  if (status_ == Status::kOnline) {
    if (!session) {
      Handle(data, nullptr);
    } else {
      StartWrite(data, session->GetRemoteEndpoint());
    }
  }
}

void UDPServer::HandleRequest(const message::Request &m,
    UDPSession::session session) {
  std::cout << "Received request, index = " << index_ << std::endl;

  if (proposed_.find(index_) == proposed_.end()) {
    auto round = std::make_shared<Round>(this, index_);
    auto command = m.GetCommand();

    clients_[index_] = session;
    rounds_[index_] = round;
    proposed_[index_] = command;

    std::cout << "Suggesting command for instance " << index_ << std::endl;
    round->Suggest(command);

    index_ += servers_.size();
  } else {
    // TODO(ljoswiak): Repropose
  }
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
  // TODO(ljoswiak): Call OnSuspect when timeout occurs?
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
