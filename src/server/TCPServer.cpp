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
      num_servers_(servers.size()) {
  std::cout << "max number of servers: " << num_servers_ << std::endl;
  // TODO(ljoswiak): This should repeat on a timer to reopen any
  // dropped connections.
  // Open connections with other servers.
  for (const auto &address : servers) {
    auto other_port = std::get<1>(address);
    // Don't try to open connection to self.
    if (other_port.compare(port) != 0) {
      std::string hostname = std::get<0>(address) + ":" + other_port;
      std::string server_name = std::get<2>(address);
      StartConnect(hostname, server_name);
    } else {
      name_ = std::get<2>(address);
    }
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
      TCPConnection::Create(this, io_context_);
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
    auto sa = message::ServerAccept(name_).Encode();
    auto encoded = message::Message(sa,
      message::MessageType::kServerSetup).Encode();
    new_connection->Deliver(encoded);
  }
}

void TCPServer::StartAccept() {
  TCPConnection::pointer new_connection =
    TCPConnection::Create(this, io_context_);

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

void TCPServer::Disconnect(TCPConnection::pointer connection)  {
  channel_.Remove(connection);
}


void TCPServer::Handle(
    const std::string &data,
    TCPConnection::pointer connection) {
  auto m = message::Message::Decode(data);
  auto type = m.GetMessageType();

  switch (type) {
    case message::MessageType::kServerSetup: {
      auto server_accept = message::ServerAccept::Decode(m.GetEncodedMessage());
      HandleServerAccept(server_accept, connection);
      break;
    }
    case message::MessageType::kRequest: {
      auto request = message::Request::Decode(m.GetEncodedMessage());
      HandleRequest(request, connection);
    }
    case message::MessageType::kReplicate: {
      auto replicate = message::Replicate::Decode(m.GetEncodedMessage());
      HandleReplicate(replicate, connection);
    }
    case message::MessageType::kReplicateAck: {
      auto replicate_ack = message::ReplicateAck::Decode(m.GetEncodedMessage());
      HandleReplicateAck(replicate_ack, connection);
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
  if (connection == nullptr) {
    Handle(data, nullptr);
  } else {
    connection->Deliver(data);
  }
}

void TCPServer::HandleServerAccept(const message::ServerAccept &m,
    TCPConnection::pointer connection) {
  std::cout << "Received ServerAccept" << std::endl;
  channel_.Add(connection);
}

void TCPServer::HandleRequest(const message::Request &m,
    TCPConnection::pointer connection) {
  // TODO(ljoswiak): Replicate before executing
  std::cout << "Received request" << std::endl;
  auto amo_response = app_->Execute(m.GetCommand());
  auto response = message::Response(amo_response).Encode();
  auto encoded =
    message::Message(response, message::MessageType::kResponse).Encode();
  Deliver(encoded, connection);
  /*
  auto replicate = message::Replicate(m.GetCommand());
  auto message = message::Message(replicate.Encode(),
    message::MessageType::kReplicate).Encode();
  Broadcast(message);
  */
}

void TCPServer::HandleReplicate(const message::Replicate &m,
    TCPConnection::pointer connection) {
  std::cout << "Received replicate" << std::endl;
  auto ack = message::ReplicateAck().Encode();
  auto message =
    message::Message(ack, message::MessageType::kReplicateAck).Encode();

  // Send response only to the server we received
  // the replicate message from.
  Deliver(message, connection);
}

void TCPServer::HandleReplicateAck(const message::ReplicateAck &m,
    TCPConnection::pointer connection) {
  std::cout << "Received replicate ack" << std::endl;
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
