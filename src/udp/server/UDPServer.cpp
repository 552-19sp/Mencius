// Copyright 2019 Lukas Joswiak, Justin Johnson, and Jack Khuu.

#include "UDPServer.hpp"

#include <iostream>

#include <boost/bind.hpp>

UDPServer::UDPServer(boost::asio::io_context &io_context, int port)
    : socket_(io_context, udp::endpoint(udp::v4(), port)) {
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
    std::cout << "Received message: " << message << std::endl;

    std::string response = "Hello, Client!";
    StartWrite(response, remote_endpoint);

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

void UDPServer::Handle(const std::string &data, udp::endpoint &endpoint) {
}

void UDPServer::Broadcast(const std::string &data) {}

void UDPServer::Deliver(const std::string &data,
    const std::string &server_name) {}

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

  boost::asio::io_context io_context;
  UDPServer server(io_context, port);
  io_context.run();

  return 1;
}
