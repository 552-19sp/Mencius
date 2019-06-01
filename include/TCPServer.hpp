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
#include "Message.hpp"
#include "Request.hpp"
#include "Replicate.hpp"
#include "ReplicateAck.hpp"
#include "TCPConnection.hpp"

using boost::asio::ip::tcp;

class TCPServer {
 public:
  TCPServer(boost::asio::io_context &io_context,
    std::string port,
    std::vector<std::tuple<std::string, std::string, std::string>> &servers);

  void Disconnect(TCPConnection::pointer connection);

  // Called by a TCPConnection instance when it receives a new message.
  void Handle(const std::string &data, TCPConnection::pointer connection);

  // Handles sending the message to all servers with open
  // connections, as well as calling local handler to
  // handle message on this server.
  void Broadcast(const std::string &data);

  // Handles delivery of message to a single server. If
  // the server is this server, handles delivery by
  // calling the appropriate handler, else delivers over
  // the network.
  void Deliver(const std::string &data,
    TCPConnection::pointer connection);

  void HandleServerAccept(const message::ServerAccept &m,
    TCPConnection::pointer connection);
  void HandleRequest(const message::Request &m,
    TCPConnection::pointer connection);
  void HandleReplicate(const message::Replicate &m,
    TCPConnection::pointer connection);
  void HandleReplicateAck(const message::ReplicateAck &m,
    TCPConnection::pointer connection);

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

  KVStore::AMOStore *app_;

  Channel channel_;

  int num_servers_;
};

#endif  // INCLUDE_TCPSERVER_HPP_
