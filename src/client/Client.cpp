// Copyright 2019 Justin Johnson, Lukas Joswiak, and Jack Khuu

#include <boost/bind.hpp>

#include "Client.hpp"
#include "Message.hpp"
#include "Utilities.hpp"

Client::Client(boost::asio::io_context &io_context)
  : stopped_(false),
    socket_(io_context),
    read_timer_(io_context),
    write_timer_(io_context) {
}

void Client::Start(tcp::resolver::iterator endpoint_iter) {
  StartConnect(endpoint_iter);

  // Start the timeout timer.
  read_timer_.async_wait(boost::bind(&Client::CheckDeadline, this));
}

void Client::Stop() {
  stopped_ = true;
  socket_.close();
  read_timer_.cancel();
  write_timer_.cancel();
}

void Client::StartConnect(tcp::resolver::iterator endpoint_iter) {
  if (endpoint_iter != tcp::resolver::iterator()) {
    std::cout << "Trying to resolve " << endpoint_iter->endpoint() << std::endl;

    // Set timeout.
    read_timer_.expires_after(boost::asio::chrono::seconds(60));

    socket_.async_connect(endpoint_iter->endpoint(),
        boost::bind(&Client::HandleConnect,
        this,
        _1,
        endpoint_iter));
  } else {
    // No more endpoints to try, shut down client.
    Stop();
  }
}

void Client::HandleConnect(const boost::system::error_code &ec,
    tcp::resolver::iterator endpoint_iter) {
  if (stopped_) {
    return;
  }

  if (!socket_.is_open()) {
    std::cout << "Connect timed out" << std::endl;
    StartConnect(++endpoint_iter);
  } else if (ec) {
    std::cout << "Connect error: " << ec.message() << std::endl;

    // Close socket from previous attempt.
    socket_.close();

    // Try next available endpoint.
    StartConnect(++endpoint_iter);
  } else {
    // Connection successfully established.
    std::cout << "Connected to " << endpoint_iter->endpoint() << std::endl;

    StartRead();
    StartWrite();
  }
}

void Client::StartRead() {
  read_timer_.expires_after(boost::asio::chrono::seconds(30));

  boost::asio::async_read_until(socket_,
      input_buffer_,
      '\n',
      boost::bind(&Client::HandleRead, this, _1));
}

void Client::HandleRead(const boost::system::error_code &ec) {
  if (stopped_) {
    return;
  }

  if (!ec) {
    std::string line;
    std::istream is(&input_buffer_);
    std::getline(is, line);

    if (!line.empty()) {
      std::cout << "Received: " << line << std::endl;
    }

    StartRead();
  } else {
    std::cout << "Error on read: " << ec.message() << std::endl;

    // TODO(ljoswiak): Does client want to kill connection on
    // when an error occurs on read? Investigate.
    // Stop();
  }
}

void Client::StartWrite() {
  if (stopped_) {
    return;
  }

  const Message msg("Hello, World!", MessageType::Reply);
  std::string m = msg.Encode();
  std::cout << "encoded: " << m << std::endl;

  boost::asio::async_write(socket_,
      boost::asio::buffer(m, m.length()),
      boost::bind(&Client::HandleWrite, this, _1));
  // TODO(ljoswiak): Can also set a deadline for message sends.
}

void Client::HandleWrite(const boost::system::error_code &ec) {
  if (stopped_) {
    return;
  }

  if (!ec) {
    write_timer_.expires_after(boost::asio::chrono::seconds(10));
    write_timer_.async_wait(boost::bind(&Client::StartWrite, this));
  } else {
    std::cout << "Error on write: " << ec.message() << std::endl;

    Stop();
  }
}

void Client::CheckDeadline() {
  if (stopped_) {
    return;
  }

  if (read_timer_.expiry() <= std::chrono::steady_clock::now()) {
    // Deadline has passed. Close socket, which cancels
    // any asynchronous operations.
    socket_.close();

    read_timer_.expires_after(boost::asio::chrono::hours(9999));
  }

  read_timer_.async_wait(boost::bind(&Client::CheckDeadline, this));
}

const char kConfigFilePath[] = "config";

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: client <operations>" << std::endl;
    return 1;
  }

  auto server_addresses = Utilities::ReadConfig(kConfigFilePath);
  std::cout << server_addresses.size() << std::endl;

  boost::asio::io_context io_context;
  tcp::resolver r(io_context);
  Client c(io_context);

  c.Start(r.resolve(tcp::resolver::query("127.0.0.1", "11111")));

  io_context.run();
}

/*
// Assume that each line in our config is <= 255 bytes.
const int MAX_CONFIG_LINE_LEN = 255;

// Assume that amount of data read from socket <= 128 bytes.
const int MAX_SOCKET_DATA_LEN = 128;

// Location of config file storing host and port pairs.
const char *CONFIG_FILE_PATH = "config";

int main(int argc, char* argv[]) {
  try {
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

    char *saveptr;  // Used for thread safety by strtok_r.
    char* token = strtok_r(unparsed_ops, ",", &saveptr);

    while (token != NULL) {
      parsed_ops.push_back(std::string(token));
      token = strtok_r(NULL, ",", &saveptr);
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
      char buf[MAX_SOCKET_DATA_LEN];
      boost::system::error_code error;

      size_t len = socket.read_some(boost::asio::buffer(buf), error);

      if (error == boost::asio::error::eof) {
        // Connection closed cleanly.
        break;
      } else if (error) {
        // Some other error.
        throw boost::system::system_error(error);
      }

      Message m;
      std::string archive_data(buf, len);
      std::istringstream archive_stream(archive_data);
      boost::archive::text_iarchive archive(archive_stream);
      archive >> m;
      std::cout << m.GetMessage() << std::endl;
    }
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}
*/
