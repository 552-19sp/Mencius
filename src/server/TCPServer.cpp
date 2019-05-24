// Copyright 2019 Lukas Joswiak, Justin Johnson, Jack Khuu.

#include <iostream>

#include <TCPServer.hpp>

TCPServer::TCPServer(boost::asio::io_context &io_context, int port)
    : io_context_(io_context),
      acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
  StartAccept();
}

void TCPServer::StartAccept() {
  TCPConnection::pointer new_connection =
    TCPConnection::Create(io_context_);

  acceptor_.async_accept(new_connection->Socket(),
    boost::bind(&TCPServer::HandleAccept,
      this,
      new_connection,
      boost::asio::placeholders::error));
}

void TCPServer::HandleAccept(TCPConnection::pointer new_connection,
    const boost::system::error_code &error) {
  if (!error) {
    new_connection->Start();
  }

  StartAccept();
}

int main(int argc, char* argv[]) {
  try {
    if (argc != 2) {
      std::cerr << "Usage: server <port>" << std::endl;
      return 1;
    }

    int port = std::stoi(argv[1]);

    boost::asio::io_context io_context;
    TCPServer server(io_context, port);

    io_context.run();
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
