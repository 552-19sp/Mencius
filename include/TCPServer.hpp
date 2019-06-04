// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#ifndef INCLUDE_TCPSERVER_HPP_
#define INCLUDE_TCPSERVER_HPP_

#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "Accept.hpp"
#include "AMOStore.hpp"
#include "Channel.hpp"
#include "Learn.hpp"
#include "Message.hpp"
#include "Request.hpp"
#include "Prepare.hpp"
#include "PrepareAck.hpp"
#include "Propose.hpp"
#include "Round.hpp"
#include "TCPConnection.hpp"

using boost::asio::ip::tcp;

class TCPServer {
 public:
  TCPServer(boost::asio::io_context &io_context,
    std::string port,
    std::vector<std::tuple<std::string, std::string, std::string>> &servers);

  std::string GetServerName() const;
  std::string GetServerName(TCPConnection::pointer connection) const;
  int GetNumServers() const {
    return num_servers_;
  }

  void Disconnect(TCPConnection::pointer connection);

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

  // Called by a TCPConnection instance when it receives a new message.
  void Handle(const std::string &data, TCPConnection::pointer connection);

  // Used to establish initial handshake between servers.
  void HandleServerAccept(const message::ServerAccept &m,
    TCPConnection::pointer connection);

  // Called when a request is received from a client.
  void HandleRequest(const message::Request &m,
    TCPConnection::pointer connection);

  void Prepare(const message::Prepare &m,
    TCPConnection::pointer connection);
  void HandlePrepareAck(const message::PrepareAck &m,
    TCPConnection::pointer connection);
  void HandlePropose(const message::Propose &m,
    TCPConnection::pointer connection);
  void HandleAccept(const message::Accept &m,
    TCPConnection::pointer connection);
  void HandleLearn(const message::Learn &m,
    TCPConnection::pointer connection);
  void HandleKillServer();

  // When receiving a proposal for instance i, skip all
  // unused instances prior to i this server owns.
  void OnSuggestion(int instance);
  void OnLearned(int instance, KVStore::AMOCommand &value);

  // TODO(ljoswiak): Clean up app_ on object destruction

 private:
  void StartConnect(const std::string &hostname,
    const std::string &server_name);
  void HandleServerConnect(const boost::system::error_code &ec,
    TCPConnection::pointer new_connection,
    std::string &server_name,
    tcp::resolver::iterator endpoint_iter);

  // Methods to accept incoming connection requests.
  void StartAccept();
  void HandleConnection(TCPConnection::pointer new_connection,
    const boost::system::error_code &error);

  std::shared_ptr<Round> GetRound(int instance);
  void CheckCommit();

  // Returns the coordinator of the given Mencius instance.
  std::string Owner(int instance);

  boost::asio::io_context &io_context_;
  std::string port_;
  std::string server_name_;
  tcp::acceptor acceptor_;
  tcp::resolver resolver_;

  KVStore::AMOStore *app_;

  Channel channel_;

  int num_servers_;

  // Mencius state.
  std::unordered_map<int, TCPConnection::pointer> clients_;
  std::unordered_map<int, std::shared_ptr<Round>> rounds_;
  std::unordered_map<int, KVStore::AMOCommand> proposed_;
  int index_;
  int expected_;
};

#endif  // INCLUDE_TCPSERVER_HPP_
