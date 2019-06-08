// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#ifndef INCLUDE_UDPSESSION_HPP_
#define INCLUDE_UDPSESSION_HPP_

#include <memory>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>

using boost::asio::ip::udp;

const int kBufferSize = 1024;

class UDPServer;

class UDPSession : public boost::enable_shared_from_this<UDPSession> {
 public:
  typedef std::shared_ptr<UDPSession> session;

  static session Create();

  UDPSession();

  udp::endpoint & GetRemoteEndpoint() {
    return remote_endpoint_;
  }

  std::array<char, kBufferSize> & GetRecvBuffer() {
    return recv_buffer_;
  }

 private:
  udp::endpoint remote_endpoint_;
  std::array<char, kBufferSize> recv_buffer_;
};

#endif  // INCLUDE_UDPSESSION_HPP_
