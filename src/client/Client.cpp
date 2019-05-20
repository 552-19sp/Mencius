// Copyright 2019 Justin Johnson, Lukas Joswiak, and Jack Khuu
#include <iostream>
#include <fstream>
#include <string>
#include <boost/array.hpp>
#include <boost/asio.hpp>

#include "Message.hpp"

using boost::asio::ip::tcp;

// Assume that each line in our config is <= 255 bytes.
const int MAX_CONFIG_LINE_LEN = 255;

// Assume that amount of data read from socket <= 128 bytes.
const int MAX_SOCKET_DATA_LEN = 128;

// Location of config file storing host and port pairs.
const char *CONFIG_FILE_PATH = "../config";

int main(int argc, char* argv[]) {
  try {
    if (argc != 2) {
      std::cerr << "Usage: client <operations>" << std::endl;
      return 1;
    }

    Message m("hello", MessageType::Request);

    // Read list of host names and ports from config file.
    std::vector<std::tuple<const char*, const char*>> serverAddresses;

    std::ifstream configFile(CONFIG_FILE_PATH);
    if (!configFile) {
      std::cout << "cannot open config file" << std::endl;
      return 1;
    }

    char buf[MAX_CONFIG_LINE_LEN];
    while (configFile) {
      configFile.getline(buf, MAX_CONFIG_LINE_LEN);
      std::string line(buf);
      if (configFile) {
        int pos = line.find_first_of(' ');
        auto host = line.substr(0, pos);
        auto port = line.substr(pos + 1);
        serverAddresses.push_back(std::make_tuple(host.c_str(), port.c_str()));
      }
    }

    // Parse operations from command argument.
    char* unparsed_ops = argv[1];
    std::vector<std::string> parsed_ops;

    char *saveptr1, *saveptr2;  // Used for thread safety by strtok_r.
    char* token = strtok_r(unparsed_ops, ",", &saveptr1);

    while (token != NULL) {
      parsed_ops.push_back(std::string(token));
      token = strtok_r(NULL, ",", &saveptr2);
    }

    // For now, print ops to cout. Eventually, they will be printed
    // to an output file after completion.
    for (int i = 0; i < parsed_ops.size(); i++) {
      std::cout << parsed_ops[i] << std::endl;
    }

    boost::asio::io_context io_context;
    tcp::resolver resolver(io_context);

    // Note: Only connect to the first server in our config, for now.
    auto host = std::get<0>(serverAddresses[0]);
    auto port = std::get<1>(serverAddresses[0]);
    tcp::resolver::results_type endpoints = resolver.resolve(host, port);

    tcp::socket socket(io_context);
    boost::asio::connect(socket, endpoints);

    std::cout << socket.remote_endpoint().port() << std::endl;

    for (;;) {
      boost::array<char, MAX_SOCKET_DATA_LEN> buf;
      boost::system::error_code error;

      size_t len = socket.read_some(boost::asio::buffer(buf), error);

      if (error == boost::asio::error::eof) {
        // Connection closed cleanly.
        break;
      } else if (error) {
        // Some other error.
        throw boost::system::system_error(error);
      }

      std::cout.write(buf.data(), len);
    }
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}
