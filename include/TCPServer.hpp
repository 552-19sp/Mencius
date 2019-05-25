// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#ifndef INCLUDE_TCPSERVER_HPP_
#define INCLUDE_TCPSERVER_HPP_

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "AMOStore.hpp"
#include "TCPConnection.hpp"

using boost::asio::ip::tcp;

class TCPServer {
 public:
  TCPServer(boost::asio::io_context &io_context, int port);

  // TODO(ljoswiak): Clean up app_ on object destruction

 private:
  void StartAccept();

  void HandleAccept(TCPConnection::pointer new_connection,
    const boost::system::error_code &error);

  boost::asio::io_context &io_context_;
  tcp::acceptor acceptor_;
  KVStore::AMOStore *app_;
};

#endif  // INCLUDE_TCPSERVER_HPP_
