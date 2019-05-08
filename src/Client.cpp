#include <iostream>
#include <boost/asio.hpp>

int main() {
    boost::asio::io_service io;
    boost::asio::deadline_timer t(io, boost::posix_time::seconds(5));
    t.wait();

    std::cout << "Hello, World!" << std::endl;

    return 0;
}
