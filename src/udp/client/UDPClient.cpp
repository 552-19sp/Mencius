// Copyright 2019 Lukas Joswiak, Justin Johnson, and Jack Khuu.

#include "UDPClient.hpp"

#include <iostream>

UDPClient::UDPClient(boost::asio::io_context &io_context,
    std::string host, std::string port)
    : socket_(io_context) {
  udp::resolver r(io_context);
  endpoint_ = *r.resolve(udp::v4(), host, port);
}

UDPClient::~UDPClient() {
  socket_.close();
}

void UDPClient::Send(const std::string &message) {
  socket_.send_to(boost::asio::buffer(message, message.size()), endpoint_);
}

int main() {
  boost::asio::io_context io_context;
  UDPClient c(io_context, "127.0.0.1", "11111");

  return 1;
}
