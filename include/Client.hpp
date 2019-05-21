// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#ifndef INCLUDE_CLIENT_HPP_
#define INCLUDE_CLIENT_HPP_

#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class Client {
 public:
  explicit Client(boost::asio::io_context &io_context);

  void Start(tcp::resolver::iterator endpoint_iter);
  void Stop();
 private:
  void StartConnect(tcp::resolver::iterator endpoint_iter);
  void HandleConnect(const boost::system::error_code &ec,
    tcp::resolver::iterator endpoint_iter);

  void StartRead();
  void HandleRead(const boost::system::error_code &ec);

  void StartWrite();
  void HandleWrite(const boost::system::error_code &ec);

  void CheckDeadline();

  bool stopped_;
  tcp::socket socket_;
  boost::asio::streambuf input_buffer_;
  boost::asio::steady_timer read_timer_;
  boost::asio::steady_timer write_timer_;
};

#endif  // INCLUDE_CLIENT_HPP_
