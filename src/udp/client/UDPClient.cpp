// Copyright 2019 Lukas Joswiak, Justin Johnson, and Jack Khuu.

#include "UDPClient.hpp"

#include <iostream>

#include <boost/bind.hpp>

#include "Action.hpp"
#include "Message.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Utilities.hpp"

const char kThreeReplicaAddr[] = "127.0.0.1";
const char kFiveReplicaAddr[] = "35.171.129.43";
const char kReplicaPort[] = "11111";

const int kRetryTimeoutMillis = 100;

UDPClient::UDPClient(boost::asio::io_context &io_context,
    const std::vector<KVStore::AMOCommand> &workload,
    int num_servers, int drop_rate, bool kill_servers)
    : socket_(io_context, udp::endpoint(udp::v4(), 0)),
      retry_timer_(io_context),
      workload_(workload),
      num_servers_(num_servers),
      server_drop_rate_(drop_rate),
      kill_servers_(kill_servers) {
  udp::resolver r(io_context);
  if (num_servers == 3) {
    remote_endpoint_ = *r.resolve(udp::v4(), kThreeReplicaAddr, kReplicaPort);
  } else if (num_servers == 5) {
    remote_endpoint_ = *r.resolve(udp::v4(), kFiveReplicaAddr, kReplicaPort);
  } else {
    std::cerr << "unsupported number of replicas" << std::endl;
  }
  std::cout << "Remote endpoint: " << remote_endpoint_ << std::endl;

  // Reverse workload (client reads starting from the end).
  std::reverse(workload_.begin(), workload_.end());

  StartRead();
  SetServerDropRate();
}

UDPClient::~UDPClient() {
  socket_.close();
}

void UDPClient::Send() {
  auto request = message::Request(*command_);
  auto message = message::Message(request.Encode(),
      message::MessageType::kRequest).Encode();
  std::cout << "Sending request to server with seq num "
      << command_->GetSeqNum() << std::endl;

  socket_.async_send_to(boost::asio::buffer(message, message.size()),
      remote_endpoint_, boost::bind(&UDPClient::HandleSend, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}

void UDPClient::HandleSend(const boost::system::error_code &ec,
    std::size_t bytes_transferred) {
  if (ec) {
    std::cerr << "Send error: " << ec.message() << std::endl;
  }
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
    auto m = message::Message::Decode(message);
    if (m.GetMessageType() == message::MessageType::kResponse) {
      auto response = message::Response::Decode(m.GetEncodedMessage());
      auto command = response.GetResponse().GetCommand();
      if (command.GetSeqNum() == (*command_).GetSeqNum()) {
        auto value = response.GetResponse().GetValue();
        std::cout << "Received reply. Original command: "
            << command << std::endl;
        std::cout << " Value: " << value << std::endl;

        retry_timer_.cancel();
        command_ = nullptr;

        ProcessWorkload();
      }
    }

    StartRead();
  } else {
    std::cerr << "Read error: " << ec.message() << std::endl;
  }
}

void UDPClient::RetryTimer(int seq_num) {
  if (command_ && command_->GetSeqNum() == seq_num) {
    // Outstading request, resend.
    std::cout << "************ Resending command *************" << std::endl;
    Send();

    retry_timer_.expires_at(retry_timer_.expiry() +
        std::chrono::milliseconds(kRetryTimeoutMillis));
    retry_timer_.async_wait(
        boost::bind(&UDPClient::RetryTimer, this, seq_num));
  }
}

void UDPClient::ProcessWorkload() {
  if (workload_.size() == 0) {
    std::cout << "no workload" << std::endl;
    exit(EXIT_SUCCESS);
  }

  command_ = &workload_.back();
  command_->SetSeqNum(command_->GetSeqNum() + 1);
  workload_.pop_back();

  retry_timer_.expires_after(
      std::chrono::milliseconds(kRetryTimeoutMillis));
  retry_timer_.async_wait(
      boost::bind(&UDPClient::RetryTimer, this, command_->GetSeqNum()));

  Send();
}

void UDPClient::SetServerDropRate() {
  std::cout << "sending server drop rate message, drop rate = "
      << server_drop_rate_ << std::endl;

  std::string drop_rate = std::to_string(server_drop_rate_);
  auto command = std::make_shared<KVStore::AMOCommand>(0, drop_rate, "",
      KVStore::Action::kSetDropRate);
  command_ = command.get();

  retry_timer_.expires_after(
      std::chrono::milliseconds(kRetryTimeoutMillis));
  retry_timer_.async_wait(
      boost::bind(&UDPClient::RetryTimer, this, command_->GetSeqNum()));

  Send();
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
  UDPClient c(io_context, workload, num_servers, server_drop_rate, kill_servers);

  io_context.run();

  return 1;
}
