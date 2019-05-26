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

#include "AMOStore.hpp"
#include "ServerAccept.hpp"

using boost::asio::ip::tcp;

class TCPConnection
  : public boost::enable_shared_from_this<TCPConnection> {
 public:
  typedef boost::shared_ptr<TCPConnection> pointer;

  static pointer Create(boost::asio::io_context &io_context,
    KVStore::AMOStore *app,
    std::unordered_map<std::string,
      TCPConnection::pointer> *server_connections);

  tcp::socket &Socket() {
    return socket_;
  }

  void Start();

  void Deliver(const std::string &message);

 private:
  explicit TCPConnection(boost::asio::io_context &io_context,
    KVStore::AMOStore *app,
    std::unordered_map<std::string,
      TCPConnection::pointer> *server_connections);

  void PrintServers();

  void Stop();
  bool Stopped() const;

  void AwaitOutput();
  void StartWrite();
  void HandleWrite(const boost::system::error_code &ec);

  void StartRead();
  void HandleRead(const boost::system::error_code &ec);

  void HandleRequest(const KVStore::AMOCommand &m);
  void HandleServerAccept(const ServerAccept &m);

  tcp::socket socket_;
  boost::asio::streambuf input_buffer_;
  std::deque<std::string> output_queue_;
  boost::asio::steady_timer non_empty_output_queue_;
  boost::asio::steady_timer servers_timer_;

  KVStore::AMOStore *app_;
  std::unordered_map<std::string, TCPConnection::pointer> *server_connections_;
};

#endif  // INCLUDE_TCPCONNECTION_HPP_
