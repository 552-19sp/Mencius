// Copyright 2019 Lukas Joswiak, Justin Johnson, and Jack Khuu.

#ifndef INCLUDE_UDPSERVER_HPP_
#define INCLUDE_UDPSERVER_HPP_

#include <boost/asio.hpp>

using boost::asio::ip::udp;

const int kBufferSize = 1024;

class UDPServer {
 public:
  UDPServer(boost::asio::io_context &io_context, int port);

  void StartRead();
  void HandleRead(const boost::system::error_code &ec,
      std::size_t bytes_transferred);

  void HandleSend(const boost::system::error_code &ec,
      std::size_t bytes_transferred);

 private:
  udp::socket socket_;
  udp::endpoint remote_endpoint_;
  std::array<char, kBufferSize> recv_buffer_;
};

#endif  // INCLUDE_UDPSERVER_HPP_
