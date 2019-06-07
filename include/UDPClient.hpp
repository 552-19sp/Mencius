// Copyright 2019 Lukas Joswiak, Justin Johnson, and Jack Khuu.

#ifndef INCLUDE_UDPCLIENT_HPP_
#define INCLUDE_UDPCLIENT_HPP_

#include <string>

#include <boost/asio.hpp>

using boost::asio::ip::udp;

class UDPClient {
 public:
  UDPClient(boost::asio::io_context &io_context,
      std::string host, std::string port);
  ~UDPClient();

  void Send(const std::string &message);

 private:
  udp::socket socket_;
  udp::endpoint endpoint_;
};

#endif  // INCLUDE_UDPCLIENT_HPP_
