// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#include "TCPServer.hpp"

#include <iostream>

#include <boost/algorithm/string.hpp>

#include "ServerAccept.hpp"
#include "Utilities.hpp"

const char kConfigFilePath[] = "config";

TCPServer::TCPServer(boost::asio::io_context &io_context,
    std::string port,
    std::vector<std::tuple<std::string, std::string>> &servers)
    : io_context_(io_context),
      port_(port),
      acceptor_(io_context, tcp::endpoint(tcp::v4(), std::stoi(port))),
      resolver_(io_context),
      num_servers_(servers.size() + 1) {
  std::cout << "max number of servers: " << num_servers_ << std::endl;
  // TODO(ljoswiak): This should repeat on a timer to reopen any
  // dropped connections.
  // Open connections with other servers.
  for (const auto &address : servers) {
    auto other_port = std::get<1>(address);
    if (other_port.compare(port) != 0) {
      // Don't try to open connection to self.
      std::string hostname = std::get<0>(address) + ":" + other_port;
      if (server_connections_.find(other_port) == server_connections_.end()) {
        StartConnect(hostname);
      }
    }
  }

  app_ = new KVStore::AMOStore();
  StartAccept();
}

void TCPServer::StartConnect(const std::string &hostname) {
  std::vector<std::string> parts;
  boost::split(parts, hostname, boost::is_any_of(":"));
  auto host = parts[0];
  auto port = parts[1];
  auto endpoint_iter = resolver_.resolve(tcp::resolver::query(host, port));
  if (endpoint_iter != tcp::resolver::iterator()) {
    std::cout << "Trying to resolve " << endpoint_iter->endpoint() << std::endl;

    TCPConnection::pointer new_connection =
      TCPConnection::Create(io_context_, app_, &server_connections_);
    new_connection->Socket().async_connect(endpoint_iter->endpoint(),
      boost::bind(&TCPServer::HandleServerAccept,
      this,
      _1,
      new_connection,
      port,
      endpoint_iter));
  } else {
    std::cerr << "error" << std::endl;
  }
}

void TCPServer::HandleServerAccept(const boost::system::error_code &ec,
    TCPConnection::pointer new_connection,
    std::string &port,
    tcp::resolver::iterator endpoint_iter) {
  if (!new_connection->Socket().is_open()) {
    std::cerr << "Connect timed out" << std::endl;
  } else if (ec) {
    std::cerr << "Connect error: " << ec.message() << std::endl;
  } else {
    // Connection successfully established.
    std::cout << "Established connection with server "
      << endpoint_iter->endpoint() << std::endl;
    server_connections_[port] = new_connection;
    new_connection->Start();

    // Send initial ServerAccept with information about this
    // server to newly connected server.
    auto sa = ServerAccept(port_).Encode();
    auto encoded = Message(sa, MessageType::ServerSetup).Encode();
    new_connection->Deliver(encoded);
  }
}

void TCPServer::StartAccept() {
  TCPConnection::pointer new_connection =
    TCPConnection::Create(io_context_, app_, &server_connections_);

  acceptor_.async_accept(new_connection->Socket(),
    boost::bind(&TCPServer::HandleAccept,
      this,
      new_connection,
      boost::asio::placeholders::error));
}

void TCPServer::HandleAccept(TCPConnection::pointer new_connection,
    const boost::system::error_code &ec) {
  std::cout << "Server handle accept "
    << new_connection->Socket().remote_endpoint() << std::endl;
  if (!ec) {
    new_connection->Start();
  }

  StartAccept();
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
