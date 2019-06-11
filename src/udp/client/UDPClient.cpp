// Copyright 2019 Lukas Joswiak, Justin Johnson, and Jack Khuu.

#include "UDPClient.hpp"

#include <iostream>

#include <boost/bind.hpp>

#include "Action.hpp"
#include "Message.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Utilities.hpp"

const int kRetryTimeoutMillis = 100;

UDPClient::UDPClient(boost::asio::io_context &io_context,
    const std::string &host, const std::string &port,
    const std::vector<KVStore::AMOCommand> &workload)
    : socket_(io_context, udp::endpoint(udp::v4(), 0)),
      retry_timer_(io_context),
      workload_(workload) {
  udp::resolver r(io_context);
  remote_endpoint_ = *r.resolve(udp::v4(), host, port);
  std::cout << "Remote endpoint: " << remote_endpoint_ << std::endl;

  // Reverse workload (client reads starting from the end).
  std::reverse(workload_.begin(), workload_.end());

  StartRead();
  ProcessWorkload();
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

  socket_.send_to(boost::asio::buffer(message, message.size()),
      remote_endpoint_);
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

void UDPClient::ProcessWorkload() {
  if (workload_.size() == 0) {
    std::cout << "no workload" << std::endl;
    exit(EXIT_SUCCESS);
  }

  command_ = &workload_.back();
  workload_.pop_back();

  retry_timer_.expires_after(
      std::chrono::milliseconds(kRetryTimeoutMillis));
  retry_timer_.async_wait(
      boost::bind(&UDPClient::RetryTimer, this, command_->GetSeqNum()));

  Send();
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

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <operations>" << std::endl;
    return 0;
  }

  auto workload = Utilities::ParseOperations(argv[1]);

  boost::asio::io_context io_context;
  UDPClient c(io_context, "127.0.0.1", "11111", workload);

  io_context.run();

  return 1;
}
