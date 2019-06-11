// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#include "TCPServer.hpp"

#include <iostream>

#include <boost/algorithm/string.hpp>

#include "Response.hpp"
#include "Utilities.hpp"

const char kConfigFilePath[] = "config";

const int kHeartbeatCheckTimeoutMillis = 100;

TCPServer::TCPServer(boost::asio::io_context &io_context,
    std::string port,
    std::vector<std::tuple<std::string, std::string, std::string>> &servers)
    : io_context_(io_context),
      port_(port),
      acceptor_(io_context, tcp::endpoint(tcp::v4(), std::stoi(port))),
      resolver_(io_context),
      heartbeat_check_timer_(io_context),
      app_(new KVStore::AMOStore()),
      servers_(servers),
      status_(Status::kOnline),
      expected_(0) {
  std::cout << "max number of servers: " << servers_.size() << std::endl;
  // TODO(ljoswiak): This should repeat on a timer to reopen any
  // dropped connections.
  // Open connections with other servers.
  int counter = 0;
  for (const auto &address : servers) {
    auto other_port = std::get<1>(address);
    // Don't try to open connection to self.
    if (other_port.compare(port) != 0) {
      std::string hostname = std::get<0>(address) + ":" + other_port;
      std::string server_name = std::get<2>(address);
      StartConnect(hostname, server_name);
    } else {
      index_ = counter;
      server_name_ = std::get<2>(address);

      std::cout << server_name_ << " starting index: " << index_ << std::endl;
    }
    counter++;
  }

  heartbeat_check_timer_.expires_after(std::chrono::seconds(7));
  heartbeat_check_timer_.async_wait(
      boost::bind(&TCPServer::HeartbeatCheckTimer, this));

  StartAccept();
}

void TCPServer::StartConnect(const std::string &hostname,
    const std::string &server_name) {
  std::vector<std::string> parts;
  boost::split(parts, hostname, boost::is_any_of(":"));
  auto host = parts[0];
  auto port = parts[1];
  auto endpoint_iter = resolver_.resolve(tcp::resolver::query(host, port));
  if (endpoint_iter != tcp::resolver::iterator()) {
    std::cout << "Trying to resolve " << endpoint_iter->endpoint() << std::endl;

    TCPConnection::pointer new_connection =
      TCPConnection::Create(this, io_context_, server_name);
    new_connection->Socket().async_connect(endpoint_iter->endpoint(),
      boost::bind(&TCPServer::HandleServerConnect,
      this,
      _1,
      new_connection,
      server_name,
      endpoint_iter));
  } else {
    std::cerr << "error" << std::endl;
  }
}

void TCPServer::HandleServerConnect(const boost::system::error_code &ec,
    TCPConnection::pointer new_connection,
    std::string &server_name,
    tcp::resolver::iterator endpoint_iter) {
  if (!new_connection->Socket().is_open()) {
    std::cerr << "Connect timed out" << std::endl;
  } else if (ec) {
    std::cerr << "Connect error: " << ec.message() << std::endl;
  } else {
    // Connection successfully established.
    std::cout << "Established connection with server "
      << endpoint_iter->endpoint() << std::endl;
    new_connection->Start();
    channel_.Add(new_connection);

    // Send initial heartbeat with information about this
    // server to newly connected server.
    auto heartbeat = message::Heartbeat(server_name_).Encode();
    auto encoded = message::Message(heartbeat,
      message::MessageType::kHeartbeat).Encode();
    new_connection->Deliver(encoded);
  }
}

void TCPServer::StartAccept() {
  TCPConnection::pointer new_connection =
    TCPConnection::Create(this, io_context_, "");

  acceptor_.async_accept(new_connection->Socket(),
    boost::bind(&TCPServer::HandleConnection,
      this,
      new_connection,
      boost::asio::placeholders::error));
}

void TCPServer::HandleConnection(TCPConnection::pointer new_connection,
    const boost::system::error_code &ec) {
  std::cout << "Server handle accept "
    << new_connection->Socket().remote_endpoint() << std::endl;
  if (!ec) {
    new_connection->Start();
  }

  StartAccept();
}

void TCPServer::Disconnect(TCPConnection::pointer connection)  {
  auto server_name = connection->GetServerName();
  channel_.Remove(connection);

  OnSuspect(server_name);
}

void TCPServer::Handle(
    const std::string &data,
    TCPConnection::pointer connection) {
  auto m = message::Message::Decode(data);
  auto type = m.GetMessageType();
  auto encoded = m.GetEncodedMessage();
  std::string server_name = "";
  if (connection) {
    server_name = connection->GetServerName();
  }

  switch (type) {
    case message::MessageType::kHeartbeat: {
      auto server_accept = message::Heartbeat::Decode(encoded);
      HandleHeartbeat(server_accept, connection);
      break;
    }
    case message::MessageType::kRequest: {
      auto request = message::Request::Decode(encoded);
      HandleRequest(request, connection);
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
    case message::MessageType::kDropRate: {
      break;  // No need to set drop rate for TCPServer.
    }
    default: {
      throw std::logic_error("unrecognized message type");
    }
  }
}

void TCPServer::Broadcast(const std::string &data) {
  if (status_ == Status::kOnline) {
    channel_.Deliver(data);
    Handle(data, nullptr);
  }
}

void TCPServer::BroadcastToOthers(const std::string &data) {
  if (status_ == Status::kOnline) {
    channel_.Deliver(data);
  }
}

void TCPServer::Deliver(const std::string &data,
    const std::string &server_name) {
  auto connection = channel_.GetConnection(server_name);
  if (status_ == Status::kOnline) {
    if (server_name.empty()) {
      Handle(data, nullptr);
    } else {
      connection->Deliver(data);
    }
  }
}

void TCPServer::Deliver(const std::string &data,
    TCPConnection::pointer connection) {
  if (status_ == Status::kOnline) {
    if (!connection) {
      Handle(data, nullptr);
    } else {
      connection->Deliver(data);
    }
  }
}

void TCPServer::HandleHeartbeat(const message::Heartbeat &m,
    TCPConnection::pointer connection) {
  std::cout << "Received Heartbeat" << std::endl;
  connection->SetServerName(m.GetServerName());
  channel_.Add(connection);
}

void TCPServer::HandleRequest(const message::Request &m,
    TCPConnection::pointer connection) {
  std::cout << "Received request, index = " << index_ << std::endl;

  if (proposed_.find(index_) == proposed_.end()) {
    // Create a new Round instance.
    auto r = std::make_shared<Round>(this, index_);
    auto command = m.GetCommand();

    clients_[index_] = connection;
    rounds_[index_] = r;
    proposed_[index_] = command;

    std::cout << "Suggesting command for instance "
        << index_ << std::endl;
    r->Suggest(command);

    index_ += servers_.size();
  }
}

std::shared_ptr<Round> TCPServer::GetRound(int instance) {
  if (!rounds_[instance]) {
    rounds_[instance] = std::make_shared<Round>(this, instance);
  }
  return rounds_[instance];
}

void TCPServer::HandlePrepare(const message::Prepare &m,
    const std::string &server_name) {
  auto round = GetRound(m.GetInstance());
  round->HandlePrepare(m, server_name);
}

void TCPServer::HandlePrepareAck(const message::PrepareAck &m,
    const std::string &server_name) {
  auto round = GetRound(m.GetInstance());
  round->HandlePrepareAck(m, server_name);
}

void TCPServer::HandlePropose(const message::Propose &m,
    const std::string &server_name) {
  auto round = GetRound(m.GetInstance());
  round->HandlePropose(m, server_name);
}

void TCPServer::TCPServer::HandleAccept(const message::Accept &m,
    const std::string &server_name) {
  auto round = GetRound(m.GetInstance());
  round->HandleAccept(m, server_name);
}

void TCPServer::HandleLearn(const message::Learn &m,
    const std::string &server_name) {
  auto round = GetRound(m.GetInstance());
  round->HandleLearn(m, server_name);
}

std::string TCPServer::Owner(int instance) {
  auto server_addresses = Utilities::ReadConfig(kConfigFilePath);
  int index = instance % servers_.size();
  return std::get<2>(server_addresses[index]);
}

std::shared_ptr<KVStore::AMOCommand> TCPServer::Learned(int instance) {
  return GetRound(instance)->GetLearnedValue();
}

void TCPServer::OnSuggestion(int instance) {
  std::cout << "OnSuggestion, instance = " << instance << std::endl;

  for (int i = index_; i < instance; i += servers_.size()) {
    std::cout << "  sending skip for instance " << i << std::endl;
    auto round = GetRound(i);
    round->Skip();

    index_ += servers_.size();
  }
}

void TCPServer::OnSuspect(const std::string &server) {
  for (int i = expected_; i < index_; i++) {
    auto owner = Owner(i);
    if (owner.compare(server) == 0 && !Learned(i)) {
      std::cout << "  sending revoke for instance " << i << std::endl;
      auto round = GetRound(i);
      round->Revoke();
    }
  }
}

void TCPServer::CheckCommit() {
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
      auto client_connection = clients_[expected_];
      if  (client_connection) {
        // If client connection is null, this value is being
        // learned on a server that did not receive the request,
        // so no need to send a reply.
        auto response = message::Response(amo_response).Encode();
        auto encoded = message::Message(response,
            message::MessageType::kResponse).Encode();
        Deliver(encoded, client_connection);
      }
    } else {
      std::cout << "  committing no-op for instance " << expected_ << std::endl;
    }

    expected_++;
  }
  std::cout << "  end of CheckCommit. expected = " << expected_ << std::endl;
}

void TCPServer::OnLearned(int instance, const KVStore::AMOCommand &value) {
  std::cout << "OnLearned, instance = " << instance << std::endl;

  auto instance_owner = Owner(instance);
  auto proposed_command = proposed_[instance];
  if (instance_owner.compare(server_name_) != 0 &&
      proposed_[instance] == value) {
    // TODO(ljoswiak): Propose value in proposed_[instance]
  }

  CheckCommit();
}

void TCPServer::HeartbeatCheckTimer() {
  std::vector<std::string> offline_servers =
      channel_.OfflineServers(servers_, server_name_);

  for (auto server_name : offline_servers) {
    OnSuspect(server_name);
  }

  heartbeat_check_timer_.expires_at(heartbeat_check_timer_.expiry() +
      std::chrono::milliseconds(kHeartbeatCheckTimeoutMillis));
  heartbeat_check_timer_.async_wait(
      boost::bind(&TCPServer::HeartbeatCheckTimer, this));
}

std::string TCPServer::GetServerName() const {
  return server_name_;
}

std::string TCPServer::GetServerName(TCPConnection::pointer connection) const {
  if (connection) {
    return connection->GetServerName();
  } else {
    return GetServerName();
  }
}

int main(int argc, char* argv[]) {
  try {
    if (argc != 2) {
      std::cerr << "Usage: server <port>" << std::endl;
      return 1;
    }

    auto server_addresses = Utilities::ReadConfig(kConfigFilePath);

    std::string port = argv[1];

    boost::asio::io_context io_context;
    TCPServer server(io_context, port, server_addresses);

    io_context.run();
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
