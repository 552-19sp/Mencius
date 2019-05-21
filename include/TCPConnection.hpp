// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#ifndef INCLUDE_TCPCONNECTION_HPP_
#define INCLUDE_TCPCONNECTION_HPP_

#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

using boost::asio::ip::tcp;

class TCPConnection
  : public boost::enable_shared_from_this<TCPConnection> {
 public:
  typedef boost::shared_ptr<TCPConnection> pointer;

  static pointer Create(boost::asio::io_context &io_context);

  tcp::socket &Socket() {
    return socket_;
  }

  void Start();

 private:
  explicit TCPConnection(boost::asio::io_context &io_context);

  void HandleWrite();

  void StartRead();
  void HandleRead(const boost::system::error_code &ec);

  tcp::socket socket_;
  boost::asio::streambuf input_buffer_;
  std::string message_;
};

#endif  // INCLUDE_TCPCONNECTION_HPP_
