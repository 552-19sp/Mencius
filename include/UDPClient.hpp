// Copyright 2019 Lukas Joswiak, Justin Johnson, and Jack Khuu.

#ifndef INCLUDE_UDPCLIENT_HPP_
#define INCLUDE_UDPCLIENT_HPP_

#include <string>

#include <boost/asio.hpp>

#include "AMOCommand.hpp"

using boost::asio::ip::udp;

const int kBufferSize = 1024;

class UDPClient {
 public:
  UDPClient(boost::asio::io_context &io_context,
      const std::string &host, const std::string &port);
  ~UDPClient();

  void Send(const KVStore::AMOCommand &command);

 private:
  void StartRead();
  void HandleRead(const boost::system::error_code &ec,
      std::size_t bytes_transferred);

  udp::socket socket_;
  udp::endpoint remote_endpoint_;
  std::array<char, kBufferSize> recv_buffer_;
};

#endif  // INCLUDE_UDPCLIENT_HPP_
