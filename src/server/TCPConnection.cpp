// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#include "TCPConnection.hpp"

#include <chrono>
#include <iostream>

#include "AMOResponse.hpp"
#include "Channel.hpp"
#include "Message.hpp"
#include "KVStore.hpp"

TCPConnection::pointer TCPConnection::Create(
    Channel &channel,
    boost::asio::io_context &io_context,
    KVStore::AMOStore *app) {
  return pointer(new TCPConnection(channel, io_context, app));
}

void TCPConnection::Start() {
  std::cout << "Client connected -- "
      << socket_.remote_endpoint() << std::endl;

  StartRead();
  AwaitOutput();
}

TCPConnection::TCPConnection(Channel &channel,
    boost::asio::io_context &io_context,
    KVStore::AMOStore *app)
    : channel_(channel),
      socket_(io_context),
      non_empty_output_queue_(io_context),
      app_(app) {
  // Set timer to max value when queue is empty.
  non_empty_output_queue_.expires_after(std::chrono::hours(9999));
}

TCPConnection::~TCPConnection() {
  std::cout << "TCPConnection destructor" << std::endl;
}

void TCPConnection::Stop() {
  std::cout << "Closing connection" << std::endl;
  channel_.Remove(shared_from_this());
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
    if (m.GetMessageType() == MessageType::Request) {
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
  /*
  auto response = app_->Execute(m).Encode();
  auto encoded = Message(response, MessageType::Response).Encode();
  */
  auto encoded = Message(m.Encode(), MessageType::Request).Encode();

  // Send to all other servers.
  channel_.Deliver(encoded);
}

void TCPConnection::HandleServerAccept(const ServerAccept &m) {
  std::cout << "Received ServerAccept" << std::endl;
  // (*server_connections_)[m.GetServerName()] = shared_from_this();
  channel_.Add(shared_from_this());
}
