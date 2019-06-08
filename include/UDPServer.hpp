// Copyright 2019 Lukas Joswiak, Justin Johnson, and Jack Khuu.

#ifndef INCLUDE_UDPSERVER_HPP_
#define INCLUDE_UDPSERVER_HPP_

#include <unordered_map>
#include <string>
#include <memory>
#include <tuple>
#include <vector>

#include <boost/asio.hpp>

#include "AMOStore.hpp"
#include "Round.hpp"
#include "Status.hpp"
#include "Server.hpp"
#include "UDPSession.hpp"

using boost::asio::ip::udp;

class UDPServer : public Server {
 public:
  UDPServer(boost::asio::io_context &io_context, int port);

  std::string GetServerName() const;
  int GetNumServers() const;
  Status GetServerStatus() const;

  void StartRead();
  void HandleRead(UDPSession::session session,
      const boost::system::error_code &ec,
      std::size_t bytes_transferred);

  void StartWrite(const std::string &data, udp::endpoint &endpoint);
  void HandleWrite(const boost::system::error_code &ec,
      std::size_t bytes_transferred);

  void Handle(const std::string &data, udp::endpoint &endpoint);

  void Broadcast(const std::string &data);
  void Deliver(const std::string &data, const std::string &server_name);

  std::string Owner(int instance);

  void OnSuggestion(int instance);
  void OnSuspect(const std::string &server);
  void OnLearned(int instance, const KVStore::AMOCommand &value);

 private:
  udp::socket socket_;
  std::string server_name_;

  KVStore::AMOStore *app_;

  std::vector<std::tuple<std::string, std::string, std::string>> servers_;
  Status status_;

  // Mencius state.
  std::unordered_map<int, udp::endpoint> clients_;
  std::unordered_map<int, std::shared_ptr<Round>> rounds_;
  std::unordered_map<int, KVStore::AMOCommand> proposed_;
  int index_;
  int expected_;
};

#endif  // INCLUDE_UDPSERVER_HPP_
