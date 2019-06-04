// Copyright 2019 Justin Johnson, Lukas Joswiak, and Jack Khuu

#include "Client.hpp"

#include <boost/bind.hpp>

#include "Action.hpp"
#include "AMOCommand.hpp"
#include "AMOResponse.hpp"
#include "DropRate.hpp"
#include "Message.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Utilities.hpp"

// TODO(jjohnson): Update this once we have the real cluster addresses
// and read them from the config here.
const char kThreeReplicaAddr[] = "35.171.129.43";
const char kFiveReplicaAddr[] = "35.171.129.43";
const char kReplicaPort[] = "11111";

Client::Client(boost::asio::io_context &io_context, int drop_rate,
    const std::vector<KVStore::AMOCommand> &workload)
    : stopped_(false),
      server_drop_rate_(drop_rate),
      socket_(io_context),
      workload_(workload),
      read_timer_(io_context),
      write_timer_(io_context) {
  // Reverse workload (client reads starting from the end).
  std::reverse(workload_.begin(), workload_.end());
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
    StartRead();
    SetServerDropRate();
    ProcessWorkload();
  }
}

void Client::ProcessWorkload() {
  if (workload_.size() == 0) {
    std::cout << "no workload" << std::endl;
    exit(EXIT_SUCCESS);
  }
  auto command = workload_.back();
  workload_.pop_back();
  StartWrite(command);
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
        std::cout << "Received reply. Original command: "
            << response.GetResponse().GetCommand() << std::endl;
        std::cout << " Value: " << value << std::endl;

        // Keep executing any remaining workload, exit if finished.
        if (workload_.size() == 0) {
          std::cout << "finished workload" << std::endl;
          exit(EXIT_SUCCESS);
        }
        auto command = workload_.back();
        workload_.pop_back();
        StartWrite(command);
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

  auto request = message::Request(command);
  auto encoded = message::Message(request.Encode(),
      message::MessageType::kRequest).Encode();
  std::cout << "Sending request to server" << std::endl;

  boost::asio::async_write(socket_,
      boost::asio::buffer(encoded),
      boost::bind(&Client::HandleWriteResult, this, _1));
  // TODO(ljoswiak): Can also set a deadline for message sends.
}

void Client::HandleWriteResult(const boost::system::error_code &ec) {
  if (stopped_) return;

  if (!ec) {
    std::cout << "Successfully wrote message" << std::endl;
    /*
    write_timer_.expires_after(boost::asio::chrono::seconds(10));
    write_timer_.async_wait(boost::bind(&Client::StartWrite, this));
    */
  } else {
    std::cerr << "Error on write: " << ec.message() << std::endl;
    Stop();
  }
}

void Client::SetServerDropRate() {
  auto request = message::DropRate(server_drop_rate_);
  auto encoded = message::Message(request.Encode(),
      message::MessageType::kDropRate).Encode();
  std::cout << "sending server drop rate message" << std::endl;

  boost::asio::async_write(socket_,
      boost::asio::buffer(encoded),
      boost::bind(&Client::HandleWriteResult, this, _1));
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

int main(int argc, char **argv) {
  if (argc != 4) {
    std::cerr << "Usage: client <number-of-servers> " <<
      " <server-drop-rate> <operations>" << std::endl;
    return 1;
  }

  int num_servers = atoi(argv[1]);
  int server_drop_rate = atoi(argv[2]);
  auto workload = Utilities::ParseOperations(argv[3]);

  boost::asio::io_context io_context;
  tcp::resolver r(io_context);
  Client c(io_context, server_drop_rate, workload);

  if (num_servers == 3) {
    c.Start(r.resolve(tcp::resolver::query(kThreeReplicaAddr, kReplicaPort)));
  } else if (num_servers == 5) {
    c.Start(r.resolve(tcp::resolver::query(kFiveReplicaAddr, kReplicaPort)));
  } else {
    std::cerr << "unsupported number of replicas" << std::endl;
    return 1;
  }

  io_context.run();
}
