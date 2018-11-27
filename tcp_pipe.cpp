#include "quoridor.hpp"
#include <boost/asio.hpp>
#include <iostream>
#include <string>
namespace asio = boost::asio;
using asio::ip::tcp;

int main()
{
    tcp::iostream s;

    // タイムアウト設定
    s.expires_from_now(boost::posix_time::seconds(10));

    s.connect("157.82.205.48", "8080");


    if (!s) {
        std::cerr << "error: " << s.error().message() << std::endl;
    } else {
        std::cerr << "Connected." << std::endl;
        Quoridor::State state;
        while (true) {
            s >> state;
            std::cout << state;
            std::string str;
            std::cin >> str;
            if (str == "MOVE") {
                int x, y;
                std::cin >> x >> y;
                s << x << " " << y << std::endl;
            } else if (str == "WALL") {
                char c;
                int x, y;
                std::cin >> c >> x >> y;
                s << c << " " << x << " " << y << std::endl;
            } else {
                std::cerr << "Invalid command: " << str << std::endl;
            }
        }
    }
}