// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#ifndef INCLUDE_TCPSERVER_HPP_
#define INCLUDE_TCPSERVER_HPP_

#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "AMOStore.hpp"
#include "Channel.hpp"
#include "Handler.hpp"
#include "Message.hpp"
#include "TCPConnection.hpp"

using boost::asio::ip::tcp;

class TCPServer {
 public:
  TCPServer(boost::asio::io_context &io_context,
    std::string port,
    std::vector<std::tuple<std::string, std::string, std::string>> &servers);

  // TODO(ljoswiak): Clean up app_ on object destruction

 private:
  void StartConnect(const std::string &hostname,
    const std::string &server_name);
  void HandleServerConnect(const boost::system::error_code &ec,
    TCPConnection::pointer new_connection,
    std::string &server_name,
    tcp::resolver::iterator endpoint_iter);

  void StartAccept();

  void HandleAccept(TCPConnection::pointer new_connection,
    const boost::system::error_code &error);

  boost::asio::io_context &io_context_;
  std::string port_;
  std::string name_;
  tcp::acceptor acceptor_;
  tcp::resolver resolver_;
  Channel channel_;
  Handler handler_;

  KVStore::AMOStore *app_;

  int num_servers_;
};

#endif  // INCLUDE_TCPSERVER_HPP_
