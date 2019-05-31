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

#include "Replicate.hpp"
#include "ReplicateAck.hpp"
#include "Request.hpp"
#include "ServerAccept.hpp"

class Channel;
class Handler;

using boost::asio::ip::tcp;

class TCPConnection
  : public boost::enable_shared_from_this<TCPConnection> {
 public:
  typedef boost::shared_ptr<TCPConnection> pointer;

  static pointer Create(Channel &channel,
    Handler &handler,
    boost::asio::io_context &io_context);

  ~TCPConnection();

  tcp::socket &Socket() {
    return socket_;
  }

  void Start();

  void Deliver(const std::string &message);

 private:
  explicit TCPConnection(Channel &channel,
    Handler &handler,
    boost::asio::io_context &io_context);

  void Stop();
  bool Stopped() const;

  void AwaitOutput();
  void StartWrite();
  void HandleWrite(const boost::system::error_code &ec);

  void StartRead();
  void HandleRead(const boost::system::error_code &ec);

  /*
  void HandleServerAccept(const message::ServerAccept &m);
  void HandleRequest(const message::Request &m);
  void HandleReplicate(const message::Replicate &m);
  void HandleReplicateAck(const message::ReplicateAck &m);
  */

  Channel &channel_;
  Handler &handler_;
  tcp::socket socket_;
  boost::asio::streambuf input_buffer_;
  std::deque<std::string> output_queue_;
  boost::asio::steady_timer non_empty_output_queue_;
};

#endif  // INCLUDE_TCPCONNECTION_HPP_
