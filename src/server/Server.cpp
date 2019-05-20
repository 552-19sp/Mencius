// Copyright 2019 Justin Johnson, Lukas Joswiak, and Jack Khuu
#include <ctime>
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

std::string make_daytime_string() {
  std::time_t now = std::time(0);
  return std::ctime(&now);
}

class tcp_connection
  : public boost::enable_shared_from_this<tcp_connection> {
 public:
  typedef boost::shared_ptr<tcp_connection> pointer;

  static pointer create(boost::asio::io_context& io_context) {
    return pointer(new tcp_connection(io_context));
  }

  tcp::socket& socket() {
    return socket_;
  }

  void start() {
    message_ = make_daytime_string();

    std::cout << "Client connected -- "
        << socket_.remote_endpoint() << std::endl;

    boost::asio::async_write(socket_, boost::asio::buffer(message_),
      boost::bind(&tcp_connection::handle_write, shared_from_this()));
  }

 private:
  explicit tcp_connection(boost::asio::io_context& io_context)
      : socket_(io_context) {}

  void handle_write() {
  }

  tcp::socket socket_;
  std::string message_;
};

class tcp_server {
 public:
  tcp_server(boost::asio::io_context& io_context, int port)
      : io_context_(io_context),
        acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
    start_accept();
  }

 private:
  void start_accept() {
    tcp_connection::pointer new_connection =
      tcp_connection::create(io_context_);

    acceptor_.async_accept(new_connection->socket(),
      boost::bind(&tcp_server::handle_accept, this, new_connection,
        boost::asio::placeholders::error));
  }

  void handle_accept(tcp_connection::pointer new_connection,
      const boost::system::error_code& error) {
    if (!error) {
      new_connection->start();
    }

    start_accept();
  }

  boost::asio::io_context& io_context_;
  tcp::acceptor acceptor_;
};

int main(int argc, char* argv[]) {
  try {
    if (argc != 2) {
      std::cerr << "Usage: server <port>" << std::endl;
      return 1;
    }

    int port = std::stoi(argv[1]);

    boost::asio::io_context io_context;
    tcp_server server(io_context, port);

    io_context.run();
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
