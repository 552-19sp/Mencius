// Copyright 2019 Lukas Joswiak, Justin Johnson, and Jack Khuu.

#include "UDPServer.hpp"

#include <iostream>

#include <boost/bind.hpp>

UDPServer::UDPServer(boost::asio::io_context &io_context, int port)
    : socket_(io_context, udp::endpoint(udp::v4(), port)) {
  StartRead();
}

void UDPServer::StartRead() {
  socket_.async_receive_from(boost::asio::buffer(recv_buffer_),
      remote_endpoint_, boost::bind(&UDPServer::HandleRead, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}

void UDPServer::HandleRead(const boost::system::error_code &ec,
    std::size_t bytes_transferred) {
  if (!ec) {
    auto message = std::string(&recv_buffer_[0],
        &recv_buffer_[0] + bytes_transferred);
    std::cout << "Received message: " << message << std::endl;

    std::string response = "Hello, Client!";

    socket_.async_send_to(boost::asio::buffer(response), remote_endpoint_,
        boost::bind(&UDPServer::HandleSend, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));

    StartRead();
  } else {
    std::cerr << "Read error: " << ec.message() << std::endl;
  }
}

void UDPServer::HandleSend(const boost::system::error_code &ec,
    std::size_t bytes_transferred) {}

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
