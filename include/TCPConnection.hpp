// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#ifndef INCLUDE_TCPCONNECTION_HPP_
#define INCLUDE_TCPCONNECTION_HPP_

#include <deque>
#include <string>
#include <unordered_map>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "Request.hpp"
#include "Status.hpp"

using boost::asio::ip::tcp;

class TCPServer;
class Handler;

class TCPConnection
  : public boost::enable_shared_from_this<TCPConnection> {
 public:
  typedef boost::shared_ptr<TCPConnection> pointer;

  static pointer Create(
    TCPServer *server,
    boost::asio::io_context &io_context,
    std::string server_name);

  ~TCPConnection();

  tcp::socket &Socket() {
    return socket_;
  }

  std::string GetServerName() const {
    return server_name_;
  }

  void SetServerName(const std::string &server_name) {
    server_name_ = server_name;
  }

  Status GetServerStatus();

  void Start();

  void Deliver(const std::string &message);

 private:
  explicit TCPConnection(
    TCPServer *server,
    boost::asio::io_context &io_context,
    std::string server_name);

  void Stop();
  bool Stopped() const;

  void AwaitOutput();
  void StartWrite();
  void HandleWrite(const boost::system::error_code &ec);

  void StartRead();
  void HandleRead(const boost::system::error_code &ec);

  TCPServer *server_;
  tcp::socket socket_;
  boost::asio::streambuf input_buffer_;
  std::deque<std::string> output_queue_;
  boost::asio::steady_timer non_empty_output_queue_;

  // Name of the server the TCP connection is connected to.
  std::string server_name_;
};

#endif  // INCLUDE_TCPCONNECTION_HPP_
