// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#include "TCPConnection.hpp"

#include <chrono>
#include <iostream>

#include "AMOResponse.hpp"
#include "Message.hpp"
#include "KVStore.hpp"

TCPConnection::pointer TCPConnection::Create(
    boost::asio::io_context &io_context,
    KVStore::AMOStore *app,
    std::unordered_map<std::string,
      TCPConnection::pointer> *server_connections) {
  return pointer(new TCPConnection(io_context, app, server_connections));
}

void TCPConnection::Start() {
  std::cout << "Client connected -- "
      << socket_.remote_endpoint() << std::endl;

  servers_timer_.async_wait(
    boost::bind(&TCPConnection::PrintServers, shared_from_this()));
  StartRead();
  AwaitOutput();
}

void TCPConnection::PrintServers() {
  std::cout << "I know about this many servers: "
    << server_connections_->size() << std::endl;
  for (auto connection : *server_connections_) {
    std::cout << "  " << connection.first << std::endl;
  }

  servers_timer_.expires_after(std::chrono::seconds(5));
  servers_timer_.async_wait(
    boost::bind(&TCPConnection::PrintServers, shared_from_this()));
}

TCPConnection::TCPConnection(boost::asio::io_context &io_context,
    KVStore::AMOStore *app,
    std::unordered_map<std::string, TCPConnection::pointer> *server_connections)
    : socket_(io_context),
      non_empty_output_queue_(io_context),
      servers_timer_(io_context),
      app_(app),
      server_connections_(server_connections) {
  // Set timer to max value when queue is empty.
  non_empty_output_queue_.expires_after(std::chrono::hours(9999));

  servers_timer_.expires_after(std::chrono::seconds(5));
}

void TCPConnection::Stop() {
  std::cout << "Closing connection" << std::endl;
  socket_.close();
  non_empty_output_queue_.cancel();
}

bool TCPConnection::Stopped() const {
  return !socket_.is_open();
}

void TCPConnection::Deliver(const std::string &message) {
  output_queue_.push_back(message);

  non_empty_output_queue_.expires_after(
    std::chrono::system_clock::duration::min());
}

void TCPConnection::AwaitOutput() {
  if (Stopped()) {
    return;
  }

  if (output_queue_.empty()) {
    // No messages ready to be sent. Go to sleep until
    // a message is ready to be sent.
    non_empty_output_queue_.expires_after(std::chrono::hours(9999));
    non_empty_output_queue_.async_wait(
        boost::bind(&TCPConnection::AwaitOutput, shared_from_this()));
  } else {
    StartWrite();
  }
}

void TCPConnection::StartWrite() {
  std::cout << "Sending message to client" << std::endl;

  boost::asio::async_write(socket_,
    boost::asio::buffer(output_queue_.front()),
    boost::bind(&TCPConnection::HandleWrite, shared_from_this(), _1));
  // TODO(ljoswiak): Can also set a deadline for message sends.
}

void TCPConnection::HandleWrite(const boost::system::error_code &ec) {
  if (Stopped()) {
    return;
  }

  if (!ec) {
    output_queue_.pop_front();

    AwaitOutput();
  } else {
    std::cerr << "Error on write: " << ec.message() << std::endl;
    Stop();
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
  if (Stopped()) {
    return;
  }

  if (!ec) {
    std::string data;
    std::istream is(&input_buffer_);
    std::getline(is, data);

    Message m = Message::Decode(data);
    std::cout << m.GetMessageType() << std::endl;
    if (m.GetMessageType() == MessageType::Request) {
      std::cout << "Received request" << std::endl;

      auto command = KVStore::AMOCommand::Decode(m.GetEncodedMessage());
      HandleRequest(command);
    } else if (m.GetMessageType() == MessageType::ServerSetup) {
      auto server_accept = ServerAccept::Decode(m.GetEncodedMessage());
      HandleServerAccept(server_accept);
    }

    StartRead();
  } else {
    std::cerr << "Error on read: " << ec.message() << std::endl;
    Stop();
  }
}

void TCPConnection::HandleRequest(const KVStore::AMOCommand &m) {
  // TODO(ljoswiak): Replicate before executing
  std::cout << "Received request" << std::endl;
  auto response = app_->Execute(m).Encode();
  auto encoded = Message(response, MessageType::Response).Encode();

  Deliver(encoded);
}

void TCPConnection::HandleServerAccept(const ServerAccept &m) {
  std::cout << "Received ServerAccept" << std::endl;
  (*server_connections_)[m.GetLocalPort()] = shared_from_this();
}
