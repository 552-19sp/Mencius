// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#include "TCPConnection.hpp"

#include <iostream>

#include "AMOResponse.hpp"
#include "Message.hpp"
#include "KVStore.hpp"

TCPConnection::pointer TCPConnection::Create(
    boost::asio::io_context &io_context,
    KVStore::AMOStore *app) {
  return pointer(new TCPConnection(io_context, app));
}

void TCPConnection::Start() {
  std::cout << "Client connected -- "
      << socket_.remote_endpoint() << std::endl;

  StartRead();
}

TCPConnection::TCPConnection(boost::asio::io_context &io_context,
    KVStore::AMOStore *app)
    : socket_(io_context),
      app_(app) {
  std::cout << "connection app addr: " << app_ << std::endl;
}

void TCPConnection::StartWrite() {
  std::cout << "Sending message to client" << std::endl;

  boost::asio::async_write(socket_,
    boost::asio::buffer(message_),
    boost::bind(&TCPConnection::HandleWrite, shared_from_this(), _1));
  // TODO(ljoswiak): Can also set a deadline for message sends.
}

void TCPConnection::HandleWrite(const boost::system::error_code &ec) {
  if (!ec) {
    /*
    write_timer_.expires_after(boost::asio::chrono::seconds(10));
    write_timer_.async_wait(boost::bind(&Client::StartWrite, this));
    */
  } else {
    std::cerr << "Error on write: " << ec.message() << std::endl;
  }
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

    Message m = Message::Decode(data);
    if (m.GetMessageType() == MessageType::Request) {
      std::cout << "Received request" << std::endl;

      auto command = KVStore::AMOCommand::Decode(m.GetEncodedMessage());

      auto response = app_->Execute(command).Encode();
      auto encoded = Message(response, MessageType::Reply).Encode();

      message_ = encoded;
      StartWrite();
    }

    StartRead();
  } else {
    std::cerr << "Error on read: " << ec.message() << std::endl;
  }
}
