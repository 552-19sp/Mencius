// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#include "UDPSession.hpp"

#include "UDPServer.hpp"

UDPSession::session UDPSession::Create() {
  return session(new UDPSession());
}

UDPSession::UDPSession() {}
