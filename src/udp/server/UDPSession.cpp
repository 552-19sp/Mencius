// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#include "UDPSession.hpp"

#include "UDPServer.hpp"

UDPSession::session UDPSession::Create() {
  return session(new UDPSession());
}

UDPSession::session UDPSession::Create(boost::asio::io_context &io_context,
    const std::string &host, const std::string &port) {
  return session(new UDPSession(io_context, host, port));
}

UDPSession::UDPSession() {}

UDPSession::UDPSession(boost::asio::io_context &io_context,
    const std::string &host, const std::string &port) {
  udp::resolver r(io_context);
  remote_endpoint_ = *r.resolve(udp::v4(), host, port);
}
