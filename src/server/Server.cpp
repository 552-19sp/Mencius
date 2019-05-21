// Copyright 2019 Justin Johnson, Lukas Joswiak, and Jack Khuu
#include <iostream>

#include "Message.hpp"
#include "TCPServer.hpp"
#include "TCPConnection.hpp"

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
