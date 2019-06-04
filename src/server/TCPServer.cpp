// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#include "TCPServer.hpp"

#include <iostream>

#include <boost/algorithm/string.hpp>

#include "ServerAccept.hpp"
#include "Response.hpp"
#include "Utilities.hpp"

const char kConfigFilePath[] = "config";

TCPServer::TCPServer(boost::asio::io_context &io_context,
    std::string port,
    std::vector<std::tuple<std::string, std::string, std::string>> &servers)
    : io_context_(io_context),
      port_(port),
      acceptor_(io_context, tcp::endpoint(tcp::v4(), std::stoi(port))),
      resolver_(io_context),
      app_(new KVStore::AMOStore()),
      num_servers_(servers.size()),
      expected_(0) {
  std::cout << "max number of servers: " << num_servers_ << std::endl;
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

    // Send initial ServerAccept with information about this
    // server to newly connected server.
    auto sa = message::ServerAccept(server_name_).Encode();
    auto encoded = message::Message(sa,
      message::MessageType::kServerSetup).Encode();
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
  channel_.Remove(connection);
}

void TCPServer::Handle(
    const std::string &data,
    TCPConnection::pointer connection) {
  auto m = message::Message::Decode(data);
  auto type = m.GetMessageType();
  auto encoded = m.GetEncodedMessage();

  switch (type) {
    case message::MessageType::kServerSetup: {
      auto server_accept = message::ServerAccept::Decode(encoded);
      HandleServerAccept(server_accept, connection);
      break;
    }
    case message::MessageType::kRequest: {
      auto request = message::Request::Decode(encoded);
      HandleRequest(request, connection);
      break;
    }
    case message::MessageType::kPropose: {
      auto propose = message::Propose::Decode(encoded);
      HandlePropose(propose, connection);
      break;
    }
    case message::MessageType::kAccept: {
      auto accept = message::Accept::Decode(encoded);
      HandleAccept(accept, connection);
      break;
    }
    case message::MessageType::kLearn: {
      auto learn = message::Learn::Decode(encoded);
      HandleLearn(learn, connection);
      break;
    }
    case message::MessageType::kDropRate: {
      break;  // No need to set drop rate for TCPServer.
    }
    case message::MessageType::kKillServer: {
      HandleKillServer();
      break;
    }
    default: {
      throw std::logic_error("unrecognized message type");
    }
  }
}

void TCPServer::Broadcast(const std::string &data) {
  channel_.Deliver(data);
  Handle(data, nullptr);
}

void TCPServer::Deliver(const std::string &data,
    TCPConnection::pointer connection) {
  if (!connection) {
    Handle(data, nullptr);
  } else {
    connection->Deliver(data);
  }
}

void TCPServer::HandleServerAccept(const message::ServerAccept &m,
    TCPConnection::pointer connection) {
  std::cout << "Received ServerAccept" << std::endl;
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

    index_ += num_servers_;
  }
}

std::shared_ptr<Round> TCPServer::GetRound(int instance) {
  if (!rounds_[instance]) {
    rounds_[instance] = std::make_shared<Round>(this, instance);
  }
  return rounds_[instance];
}

void TCPServer::Prepare(const message::Prepare &m,
    TCPConnection::pointer connection) {
}

void TCPServer::HandlePrepareAck(const message::PrepareAck &m,
    TCPConnection::pointer connection) {
}

void TCPServer::HandlePropose(const message::Propose &m,
    TCPConnection::pointer connection) {
  auto round = GetRound(m.GetInstance());
  round->HandlePropose(m, connection);
}

void TCPServer::TCPServer::HandleAccept(const message::Accept &m,
    TCPConnection::pointer connection) {
  auto round = GetRound(m.GetInstance());
  round->HandleAccept(m, connection);
}

void TCPServer::HandleLearn(const message::Learn &m,
    TCPConnection::pointer connection) {
  auto round = GetRound(m.GetInstance());
  round->HandleLearn(m, connection);
}

std::string TCPServer::Owner(int instance) {
  auto server_addresses = Utilities::ReadConfig(kConfigFilePath);
  int index = instance % num_servers_;
  return std::get<2>(server_addresses[index]);
}

void TCPServer::OnSuggestion(int instance) {
  std::cout << "OnSuggestion, instance = " << instance << std::endl;

  for (int i = index_; i < instance; i += num_servers_) {
    std::cout << "  sending skip for instance " << i << std::endl;
    auto round = GetRound(i);
    round->Skip();

    index_ += num_servers_;
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

void TCPServer::OnLearned(int instance, KVStore::AMOCommand &value) {
  std::cout << "OnLearned, instance = " << instance << std::endl;

  auto instance_owner = Owner(instance);
  auto proposed_command = proposed_[instance];
  if (instance_owner.compare(server_name_) != 0 &&
      proposed_[instance] == value) {
    // TODO(ljoswiak): Propose value in proposed_[instance]
  }

  CheckCommit();
}

// TODO(jjohnson): Figure out how to kill a TCP server.
void TCPServer::HandleKillServer() {}

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
