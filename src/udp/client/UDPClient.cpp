// Copyright 2019 Lukas Joswiak, Justin Johnson, and Jack Khuu.

#include "UDPClient.hpp"

#include <iostream>

#include <boost/bind.hpp>

#include "Action.hpp"
#include "Message.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Utilities.hpp"

UDPClient::UDPClient(boost::asio::io_context &io_context,
    const std::string &host, const std::string &port,
    const std::vector<KVStore::AMOCommand> &workload)
    : socket_(io_context, udp::endpoint(udp::v4(), 0)),
      workload_(workload) {
  udp::resolver r(io_context);
  remote_endpoint_ = *r.resolve(udp::v4(), host, port);
  std::cout << "Remote endpoint: " << remote_endpoint_ << std::endl;

  StartRead();
  ProcessWorkload();
}

UDPClient::~UDPClient() {
  socket_.close();
}

void UDPClient::Send(const KVStore::AMOCommand &command) {
  auto request = message::Request(command);
  auto message = message::Message(request.Encode(),
      message::MessageType::kRequest).Encode();
  std::cout << "Sending request to server" << std::endl;

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
      auto value = response.GetResponse().GetValue();
      std::cout << "Received reply. Original command: "
          << response.GetResponse().GetCommand() << std::endl;
      std::cout << " Value: " << value << std::endl;

      ProcessWorkload();
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

  auto command = workload_.back();
  workload_.pop_back();
  Send(command);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <operations>" << std::endl;
    return 0;
  }

  auto workload = Utilities::ParseOperations(argv[1]);

  boost::asio::io_context io_context;
  UDPClient c(io_context, "127.0.0.1", "11111", workload);

  auto command = KVStore::AMOCommand(0, "foo", "bar",
      KVStore::Action::kPut);
  c.Send(command);

  io_context.run();

  return 1;
}
