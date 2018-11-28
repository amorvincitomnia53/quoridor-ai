#include "quoridor.hpp"
#include <boost/asio.hpp>
#include <iostream>
#include <sstream>
#include <string>
namespace asio = boost::asio;
using asio::ip::tcp;

int main(int argc, const char** argv)
{

    if (argc != 2) {
        std::cerr << "Usage: tcp_pipe <Server IP Address>" << std::endl;
        return 1;
    }

    std::cerr << "Connecing to " << argv[1] << "..." << std::endl;
    boost::asio::io_service io_service;
    tcp::iostream s(argv[1], "8080");
    if (!s) {
        std::cerr << "error: " << s.error().message() << std::endl;
    } else {
        std::cerr << "Connected." << std::endl;
        int is_flipped;
        s >> is_flipped;
        std::cerr << "You are " << (is_flipped == 1 ? "PLAYER 2" : "PLAYER 1") << "." << std::endl;

        Quoridor::State state = {};
        std::stringstream ss;
        while (true) {
            s >> state;
            if (is_flipped == 1) {
                state = state.flipPlayers();
            }
            std::cout << state;
            std::string str;
            std::cin >> str;
            if (str == "MOVE") {
                int x, y;
                std::cin >> x >> y;

                if (is_flipped == 1) {
                    std::cerr << (Quoridor::N - 1 - x) << " " << (Quoridor::N - 1 - y) << std::endl;
                    ss << (Quoridor::N - 1 - x) << " " << (Quoridor::N - 1 - y);  //<< std::endl;
                } else {
                    std::cerr << x << " " << y << std::endl;
                    ss << x << " " << y;  //<< std::endl;
                }
                s << ss.rdbuf();
                std::cerr << ss.rdbuf();
            } else if (str == "WALL") {
                char c;
                int x, y;
                std::cin >> c >> x >> y;
                if (is_flipped == 1) {
                    std::cerr << " " << (Quoridor::N - 2 - x) << " " << (Quoridor::N - 2 - y) << " " << c << std::endl;
                    ss << (Quoridor::N - 2 - x) << " " << (Quoridor::N - 2 - y) << " " << c;  //<< std::endl;
                } else {
                    std::cerr << x << " " << y << " " << c << std::endl;
                    ss << x << " " << y << " " << c;  // << std::endl;
                }
                s << ss.rdbuf();
                //                std::cerr << ss.rdbuf();
            } else {
                std::cerr << "Invalid command: " << str << std::endl;
            }
        }
    }
}