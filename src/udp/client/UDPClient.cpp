// Copyright 2019 Lukas Joswiak, Justin Johnson, and Jack Khuu.

#include "UDPClient.hpp"

#include <iostream>

#include <boost/bind.hpp>

UDPClient::UDPClient(boost::asio::io_context &io_context,
    const std::string &host, const std::string &port)
    : socket_(io_context, udp::endpoint(udp::v4(), 0)) {
  udp::resolver r(io_context);
  remote_endpoint_ = *r.resolve(udp::v4(), host, port);
  std::cout << "Remote endpoint: " << remote_endpoint_ << std::endl;

  StartRead();
}

UDPClient::~UDPClient() {
  socket_.close();
}

void UDPClient::Send(const std::string &message) {
  socket_.send_to(boost::asio::buffer(message, message.size()),
      remote_endpoint_);
}

void UDPClient::StartRead() {
  socket_.async_receive_from(boost::asio::buffer(recv_buffer_),
      remote_endpoint_, boost::bind(&UDPClient::HandleRead, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}

void UDPClient::HandleRead(const boost::system::error_code &ec,
    std::size_t bytes_transferred) {
  if (!ec) {
    auto message = std::string(&recv_buffer_[0],
        &recv_buffer_[0] + bytes_transferred);
    std::cout << "Received message: " << message << std::endl;

    StartRead();
  } else {
    std::cerr << "Read error: " << ec.message() << std::endl;
  }
}

int main() {
  boost::asio::io_context io_context;
  UDPClient c(io_context, "127.0.0.1", "11111");

  c.Send("Hello, World!");

  io_context.run();

  return 1;
}
