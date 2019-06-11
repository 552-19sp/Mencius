// Copyright 2019 Lukas Joswiak, Justin Johnson, and Jack Khuu.

#ifndef INCLUDE_UDPCLIENT_HPP_
#define INCLUDE_UDPCLIENT_HPP_

#include <string>
#include <vector>

#include <boost/asio.hpp>

#include "AMOCommand.hpp"

using boost::asio::ip::udp;

const int kBufferSize = 1024;

class UDPClient {
 public:
  UDPClient(boost::asio::io_context &io_context,
      const std::string &host, const std::string &port,
      const std::vector<KVStore::AMOCommand> &workload);
  ~UDPClient();

  void Send();

 private:
  void StartRead();
  void HandleRead(const boost::system::error_code &ec,
      std::size_t bytes_transferred);

  void ProcessWorkload();

  void RetryTimer(int seq_num);

  udp::socket socket_;
  udp::endpoint remote_endpoint_;
  boost::asio::steady_timer retry_timer_;
  std::array<char, kBufferSize> recv_buffer_;
  std::vector<KVStore::AMOCommand> workload_;

  KVStore::AMOCommand *command_;
};

#endif  // INCLUDE_UDPCLIENT_HPP_
