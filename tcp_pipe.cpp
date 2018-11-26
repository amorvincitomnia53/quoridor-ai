#include <boost/asio.hpp>
#include <iostream>

namespace asio = boost::asio;
using asio::ip::tcp;

int main()
{
    asio::io_service io_service;
    tcp::socket socket(io_service);

    boost::system::error_code error;
    socket.connect(tcp::endpoint(asio::ip::address::from_string("127.0.0.1"), 31400), error);

    if (error) {
        std::cerr << "connect failed : " << error.message() << std::endl;
    } else {
        std::cout << "connected" << std::endl;
    }
}