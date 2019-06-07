// Copyright 2019 Justin Johnson, Lukas Joswiak, and Jack Khuu

#include "TCPClient.hpp"

#include <chrono>

#include <boost/bind.hpp>
#include <boost/chrono.hpp>
#include <boost/thread.hpp>

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

TCPClient::TCPClient(boost::asio::io_context &io_context, int num_servers,
    int drop_rate,  bool kill_servers,
    const std::vector<KVStore::AMOCommand> &workload)
    : stopped_(false),
      num_servers_(num_servers),
      server_drop_rate_(drop_rate),
      kill_servers_randomly_(kill_servers),
      socket_(io_context),
      workload_(workload),
      read_timer_(io_context),
      write_timer_(io_context) {
  // Reverse workload (client reads starting from the end).
  std::reverse(workload_.begin(), workload_.end());
}

void TCPClient::Start(tcp::resolver::iterator endpoint_iter) {
  StartConnect(endpoint_iter);

  // Start the timeout timer.
  read_timer_.async_wait(boost::bind(&TCPClient::CheckDeadline, this));
}

void TCPClient::Stop() {
  stopped_ = true;
  socket_.close();
  read_timer_.cancel();
  write_timer_.cancel();
}

void TCPClient::StartConnect(tcp::resolver::iterator endpoint_iter) {
  if (endpoint_iter != tcp::resolver::iterator()) {
    std::cout << "Trying to resolve " << endpoint_iter->endpoint() << std::endl;

    // Set timeout.
    read_timer_.expires_after(boost::asio::chrono::seconds(60));

    socket_.async_connect(endpoint_iter->endpoint(),
      boost::bind(&TCPClient::HandleConnect,
      this,
      _1,
      endpoint_iter));
  } else {
    // No more endpoints to try, shut down client.
    Stop();
  }
}

void TCPClient::HandleConnect(const boost::system::error_code &ec,
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

    // Ensure all servers are alive.
    for (int i = 1; i <= num_servers_; i++) {
      std::string server = "server" + std::to_string(i);
      auto revive = KVStore::AMOCommand(0, server, "",
          KVStore::Action::kReviveServer);
      StartWrite(revive);
    }

    SetServerDropRate();
    ProcessWorkload();
  }
}

void TCPClient::ProcessWorkload() {
  if (workload_.size() == 0) {
    std::cout << "no workload" << std::endl;
    exit(EXIT_SUCCESS);
  }

  auto command = workload_.back();
  workload_.pop_back();
  StartWrite(command);
}

void TCPClient::StartRead() {
  read_timer_.expires_after(boost::asio::chrono::seconds(30));

  boost::asio::async_read_until(socket_,
      input_buffer_,
      '\n',
      boost::bind(&TCPClient::HandleRead, this, _1));
}

void TCPClient::HandleRead(const boost::system::error_code &ec) {
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

void TCPClient::StartWrite(KVStore::AMOCommand command) {
  if (stopped_) return;

  auto request = message::Request(command);
  auto encoded = message::Message(request.Encode(),
      message::MessageType::kRequest).Encode();
  std::cout << "Sending request to server" << std::endl;

  boost::asio::async_write(socket_,
      boost::asio::buffer(encoded),
      boost::bind(&TCPClient::HandleWriteResult, this, _1));
  // TODO(ljoswiak): Can also set a deadline for message sends.
}

void TCPClient::HandleWriteResult(const boost::system::error_code &ec) {
  if (stopped_) return;

  if (!ec) {
    std::cout << "Successfully wrote message" << std::endl;
    /*
    write_timer_.expires_after(boost::asio::chrono::seconds(10));
    write_timer_.async_wait(boost::bind(&TCPClient::StartWrite, this));
    */
  } else {
    std::cerr << "Error on write: " << ec.message() << std::endl;
    Stop();
  }
}

void TCPClient::SetServerDropRate() {
  auto request = message::DropRate(server_drop_rate_);
  auto encoded = message::Message(request.Encode(),
      message::MessageType::kDropRate).Encode();
  std::cout << "sending server drop rate message" << std::endl;

  boost::asio::async_write(socket_,
      boost::asio::buffer(encoded),
      boost::bind(&TCPClient::HandleWriteResult, this, _1));
}

void TCPClient::CheckDeadline() {
  if (stopped_) {
    return;
  }

  if (read_timer_.expiry() <= std::chrono::steady_clock::now()) {
    // Deadline has passed. Close socket, which cancels
    // any asynchronous operations.
    socket_.close();

    read_timer_.expires_after(boost::asio::chrono::hours(9999));
  }

  read_timer_.async_wait(boost::bind(&TCPClient::CheckDeadline, this));
}

int main(int argc, char **argv) {
  if (argc != 5) {
    std::cerr << "Usage: " << argv[0] << " <number-of-servers> " <<
      " <server-drop-rate> <random-failure-bit> <operations>" << std::endl;
    return 0;
  }

  int num_servers = atoi(argv[1]);
  int server_drop_rate = atoi(argv[2]);
  bool kill_servers = atoi(argv[3]) == 1;
  auto workload = Utilities::ParseOperations(argv[4]);

  boost::asio::io_context io_context;
  tcp::resolver r(io_context);
  TCPClient c(io_context, num_servers, server_drop_rate, kill_servers,
      workload);

  if (num_servers == 3) {
    c.Start(r.resolve(tcp::resolver::query(kThreeReplicaAddr, kReplicaPort)));
  } else if (num_servers == 5) {
    c.Start(r.resolve(tcp::resolver::query(kFiveReplicaAddr, kReplicaPort)));
  } else {
    std::cerr << "unsupported number of replicas" << std::endl;
    return 1;
  }

  io_context.run();
  return 1;
}
