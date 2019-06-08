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
      status_(Status::kOnline) {
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
    auto remote_endpoint = session->GetRemoteEndpoint();
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
    std::size_t bytes_transferred) {
  std::cout << "Wrote " << bytes_transferred << " bytes" << std::endl;
}

void UDPServer::Handle(const std::string &data, UDPSession::session session) {
  auto m = message::Message::Decode(data);
  auto type = m.GetMessageType();
  auto encoded = m.GetEncodedMessage();

  switch (type) {
    case message::MessageType::kRequest: {
      auto request = message::Request::Decode(encoded);
      HandleRequest(request, session);
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

std::string UDPServer::Owner(int instance) {}

void UDPServer::OnSuggestion(int instance) {}

void UDPServer::OnSuspect(const std::string &server) {}

void UDPServer::OnLearned(int instance, const KVStore::AMOCommand &value) {}

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
