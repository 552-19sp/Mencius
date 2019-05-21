// Copyright 2019 Justin Johnson, Lukas Joswiak, and Jack Khuu

#include <boost/bind.hpp>

#include "Client.hpp"
#include "Message.hpp"
#include "Utilities.hpp"

Client::Client(boost::asio::io_context &io_context)
  : stopped_(false),
    socket_(io_context),
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
    std::cout << "Connect timed out" << std::endl;
    StartConnect(++endpoint_iter);
  } else if (ec) {
    std::cout << "Connect error: " << ec.message() << std::endl;

    // Close socket from previous attempt.
    socket_.close();

    // Try next available endpoint.
    StartConnect(++endpoint_iter);
  } else {
    // Connection successfully established.
    std::cout << "Connected to " << endpoint_iter->endpoint() << std::endl;

    StartRead();
    StartWrite();
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
    std::string line;
    std::istream is(&input_buffer_);
    std::getline(is, line);

    if (!line.empty()) {
      std::cout << "Received: " << line << std::endl;
    }

    StartRead();
  } else {
    std::cout << "Error on read: " << ec.message() << std::endl;

    // TODO(ljoswiak): Does client want to kill connection on
    // when an error occurs on read? Investigate.
    // Stop();
  }
}

void Client::StartWrite() {
  if (stopped_) {
    return;
  }

  const Message msg("Hello, World!", MessageType::Reply);
  std::string m = msg.Encode();
  std::cout << "encoded: " << m << std::endl;

  boost::asio::async_write(socket_,
      boost::asio::buffer(m, m.length()),
      boost::bind(&Client::HandleWrite, this, _1));
  // TODO(ljoswiak): Can also set a deadline for message sends.
}

void Client::HandleWrite(const boost::system::error_code &ec) {
  if (stopped_) {
    return;
  }

  if (!ec) {
    write_timer_.expires_after(boost::asio::chrono::seconds(10));
    write_timer_.async_wait(boost::bind(&Client::StartWrite, this));
  } else {
    std::cout << "Error on write: " << ec.message() << std::endl;

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

  auto parsed_ops = Utilities::ParseOperations(argv[1]);
  // For now, print ops to cout. Eventually, they will be printed
  // to an output file after completion.
  for (int i = 0; i < parsed_ops.size(); i++) {
    std::cout << parsed_ops[i] << std::endl;
  }

  boost::asio::io_context io_context;
  tcp::resolver r(io_context);
  Client c(io_context);

  c.Start(r.resolve(tcp::resolver::query("127.0.0.1", "11111")));

  io_context.run();
}
