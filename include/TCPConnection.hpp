// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#ifndef INCLUDE_TCPCONNECTION_HPP_
#define INCLUDE_TCPCONNECTION_HPP_

#include <string>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "AMOStore.hpp"

using boost::asio::ip::tcp;

class TCPConnection
  : public boost::enable_shared_from_this<TCPConnection> {
 public:
  typedef boost::shared_ptr<TCPConnection> pointer;

  static pointer Create(boost::asio::io_context &io_context,
    KVStore::AMOStore *app);

  tcp::socket &Socket() {
    return socket_;
  }

  void Start();

 private:
  explicit TCPConnection(boost::asio::io_context &io_context,
    KVStore::AMOStore *app);

  void StartWrite();
  void HandleWrite(const boost::system::error_code &ec);

  void StartRead();
  void HandleRead(const boost::system::error_code &ec);

  tcp::socket socket_;
  boost::asio::streambuf input_buffer_;
  std::string message_;
  KVStore::AMOStore *app_;
};

#endif  // INCLUDE_TCPCONNECTION_HPP_
