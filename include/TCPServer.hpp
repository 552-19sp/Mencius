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
#include "Status.hpp"
#include "Server.hpp"
#include "TCPConnection.hpp"

using boost::asio::ip::tcp;

class TCPServer : public Server {
 public:
  TCPServer(boost::asio::io_context &io_context,
    std::string port,
    std::vector<std::tuple<std::string, std::string, std::string>> &servers);

  std::string GetServerName() const;
  std::string GetServerName(TCPConnection::pointer connection) const;
  int GetNumServers() const {
    return servers_.size();
  }

  Status GetServerStatus() const {
    return status_;
  }

  void Disconnect(TCPConnection::pointer connection);

  void Broadcast(const std::string &data);

  // Broadcast a message to all servers other than the
  // sending server.
  void BroadcastToOthers(const std::string &data);

  void Deliver(const std::string &data, const std::string &server_name);

  // Called by a TCPConnection instance when it receives a new message.
  void Handle(const std::string &data, TCPConnection::pointer connection);

  // Used to establish initial handshake between servers.
  void HandleServerAccept(const message::ServerAccept &m,
    TCPConnection::pointer connection);

  // Called when a request is received from a client.
  void HandleRequest(const message::Request &m,
    TCPConnection::pointer connection);

  void HandlePrepare(const message::Prepare &m,
    const std::string &server_name);
  void HandlePrepareAck(const message::PrepareAck &m,
    const std::string &server_name);
  void HandlePropose(const message::Propose &m,
    const std::string &server_name);
  void HandleAccept(const message::Accept &m,
    const std::string &server_name);
  void HandleLearn(const message::Learn &m,
    const std::string &server_name);

  std::string Owner(int instance);

  void OnSuggestion(int instance);
  void OnSuspect(const std::string &server);
  void OnLearned(int instance, const KVStore::AMOCommand &value);

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

  // Deliver data along a given TCPConnection.
  void Deliver(const std::string &data,
    TCPConnection::pointer connection);

  std::shared_ptr<Round> GetRound(int instance);
  void CheckCommit();

  // Timer handlers.
  void HeartbeatCheckTimer();

  // Returns the learned value of the given instance, or
  // nullptr if no learned value exists.
  std::shared_ptr<KVStore::AMOCommand> Learned(int instance);

  boost::asio::io_context &io_context_;
  std::string port_;
  std::string server_name_;
  tcp::acceptor acceptor_;
  tcp::resolver resolver_;
  boost::asio::steady_timer heartbeat_check_timer_;

  KVStore::AMOStore *app_;

  Channel channel_;

  std::vector<std::tuple<std::string, std::string, std::string>> servers_;
  Status status_;

  // Mencius state.
  std::unordered_map<int, TCPConnection::pointer> clients_;
  std::unordered_map<int, std::shared_ptr<Round>> rounds_;
  std::unordered_map<int, KVStore::AMOCommand> proposed_;
  int index_;
  int expected_;
};

#endif  // INCLUDE_TCPSERVER_HPP_
