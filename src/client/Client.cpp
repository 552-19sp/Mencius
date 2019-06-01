// Copyright 2019 Justin Johnson, Lukas Joswiak, and Jack Khuu

#include "Client.hpp"

#include <boost/bind.hpp>

#include "Action.hpp"
#include "AMOCommand.hpp"
#include "AMOResponse.hpp"
#include "Message.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Utilities.hpp"

Client::Client(boost::asio::io_context &io_context,
  const std::vector<KVStore::AMOCommand> &workload)
  : stopped_(false),
    socket_(io_context),
    workload_(workload),
    read_timer_(io_context),
    write_timer_(io_context) {
}

void Client::Start(tcp::resolver::iterator endpoint_iter) {
  StartConnect(endpoint_iter);

  // Start the timeout timer.
  read_timer_.async_wait(boost::bind(&Client::CheckDeadline, this));
}

void Client::Stop() {
  stopped_ = true;
  socket_.close();
  read_timer_.cancel();
  write_timer_.cancel();
}

void Client::StartConnect(tcp::resolver::iterator endpoint_iter) {
  if (endpoint_iter != tcp::resolver::iterator()) {
    std::cout << "Trying to resolve " << endpoint_iter->endpoint() << std::endl;

    // Set timeout.
    read_timer_.expires_after(boost::asio::chrono::seconds(60));

    socket_.async_connect(endpoint_iter->endpoint(),
      boost::bind(&Client::HandleConnect,
      this,
      _1,
      endpoint_iter));
  } else {
    // No more endpoints to try, shut down client.
    Stop();
  }
}

void Client::HandleConnect(const boost::system::error_code &ec,
    tcp::resolver::iterator endpoint_iter) {
  if (stopped_) {
    return;
  }

  if (!socket_.is_open()) {
    std::cerr << "Connect timed out" << std::endl;
    StartConnect(++endpoint_iter);
  } else if (ec) {
    std::cerr << "Connect error: " << ec.message() << std::endl;

    // Close socket from previous attempt.
    socket_.close();

    // Try next available endpoint.
    StartConnect(++endpoint_iter);
  } else {
    // Connection successfully established.
    std::cout << "Connected to " << endpoint_iter->endpoint() << std::endl;
    ProcessWorkload();
  }
}

void Client::ProcessWorkload() {
  for (auto &command : workload_) {
    if (command.GetAction() == KVStore::Action::PUT) {
      StartWrite(command);
    } else {
      // TODO(jjohnson): handle other operation types here.
    }
  }
}

void Client::StartRead() {
  read_timer_.expires_after(boost::asio::chrono::seconds(30));

  boost::asio::async_read_until(socket_,
      input_buffer_,
      '\n',
      boost::bind(&Client::HandleRead, this, _1));
}

void Client::HandleRead(const boost::system::error_code &ec) {
  if (stopped_) {
    return;
  }

  if (!ec) {
    std::string data;
    std::istream is(&input_buffer_);
    std::getline(is, data);

    if (!data.empty()) {
      const auto const_data = std::string(data);
      auto m = message::Message::Decode(const_data);
      if (m.GetMessageType() == message::MessageType::kResponse) {
        auto response = message::Response::Decode(m.GetEncodedMessage());

        auto value = response.GetResponse().GetValue();
        std::cout << "Received reply. Value: " << value << std::endl;
      }
    }

    StartRead();
  } else {
    std::cerr << "Error on read: " << ec.message() << std::endl;

    // TODO(ljoswiak): Does client want to kill connection on
    // when an error occurs on read? Investigate.
    // Stop();
  }
}

void Client::StartWrite(KVStore::AMOCommand command) {
  if (stopped_) return;

  last_request_ = std::make_shared<message::Request>(message::Request(command));
  last_response_ = nullptr;
  auto encoded = message::Message(last_request_->Encode(),
    message::MessageType::kRequest).Encode();
  std::cout << "Sending request to server" << std::endl;

  boost::asio::async_write(socket_,
      boost::asio::buffer(encoded),
      boost::bind(&Client::HandleWriteResult, this, _1, command));
  // TODO(ljoswiak): Can also set a deadline for message sends.
}

void Client::HandleWriteResult(const boost::system::error_code &ec,
    KVStore::AMOCommand command) {
  if (stopped_) return;

  if (!ec) {
    // TODO(jjohnson): Write this to a shared output file.
    std::cout << "Got response to write" << std::endl;
    /*
    write_timer_.expires_after(boost::asio::chrono::seconds(10));
    write_timer_.async_wait(boost::bind(&Client::StartWrite, this));
    */
  } else {
    std::cerr << "Error on write: " << ec.message() << std::endl;
    Stop();
  }
}

void Client::CheckDeadline() {
  if (stopped_) {
    return;
  }

  if (read_timer_.expiry() <= std::chrono::steady_clock::now()) {
    // Deadline has passed. Close socket, which cancels
    // any asynchronous operations.
    socket_.close();

    read_timer_.expires_after(boost::asio::chrono::hours(9999));
  }

  read_timer_.async_wait(boost::bind(&Client::CheckDeadline, this));
}

const char kConfigFilePath[] = "config";

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: client <operations>" << std::endl;
    return 1;
  }

  auto server_addresses = Utilities::ReadConfig(kConfigFilePath);
  auto workload = Utilities::ParseOperations(argv[1]);

  boost::asio::io_context io_context;
  tcp::resolver r(io_context);
  Client c(io_context, workload);

  c.Start(r.resolve(tcp::resolver::query("127.0.0.1", "11111")));

  io_context.run();
}
