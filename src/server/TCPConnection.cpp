// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#include <iostream>

#include "Message.hpp"
#include "TCPConnection.hpp"

Message decode(std::string data) {
  Message m;
  std::istringstream archive_stream(data);
  boost::archive::text_iarchive archive(archive_stream);
  archive >> m;
  return m;
}

TCPConnection::pointer TCPConnection::Create(
    boost::asio::io_context &io_context) {
  return pointer(new TCPConnection(io_context));
}

void TCPConnection::Start() {
  message_ = "hello from server";

  std::cout << "Client connected -- "
      << socket_.remote_endpoint() << std::endl;

  // Separate into write function
  boost::asio::async_write(socket_, boost::asio::buffer(message_),
    boost::bind(&TCPConnection::HandleWrite, shared_from_this()));

  StartRead();
}

TCPConnection::TCPConnection(boost::asio::io_context &io_context)
    : socket_(io_context) {
}

void TCPConnection::HandleWrite() {
}

void TCPConnection::StartRead() {
  // TODO(ljoswiak): Add timeout

  boost::asio::async_read_until(socket_,
    input_buffer_,
    '\n',
    boost::bind(&TCPConnection::HandleRead, shared_from_this(), _1));
}

void TCPConnection::HandleRead(const boost::system::error_code &ec) {
  if (!ec) {
    std::string data;
    std::istream is(&input_buffer_);
    std::getline(is, data);

    std::cout << "Received serialized message: " << data << std::endl;

    Message m = decode(data);
    std::cout << "Decoded message: " << m.GetMessage() << std::endl;

    StartRead();
  } else {
    std::cout << "Error on read: " << ec.message() << std::endl;
  }
}
