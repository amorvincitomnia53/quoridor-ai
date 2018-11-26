#include "negaalpha.hpp"
#include "quoridor.hpp"
#include <algorithm>
#include <chrono>
#include <csignal>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>


int main(int argc, const char** argv)
{


    std::signal(SIGABRT, [](int) { exit(0); });
    std::signal(SIGINT, [](int) { exit(0); });

    if (argc != 5) {
        std::cerr << "Usage: interactive <out-path1> <in-path1> <out-path2> <in-path2>" << std::endl;
        return 1;
    }

    std::array<std::ofstream, 2> in{std::ofstream{argv[2]}, std::ofstream{argv[4]}};
    std::array<std::ifstream, 2> out{std::ifstream{argv[1]}, std::ifstream{argv[3]}};


    std::vector<Quoridor::State> undo_buffer;
    Quoridor::State s = Quoridor::State::initialState();


    int turn = 0;
    while (true) {
    back:

        std::cout << "====================================" << std::endl;
        std::cout << "====================================" << std::endl;
        std::cout << "TURN: " << turn << std::endl;
        std::cout << s << std::endl;
        std::cout << s.pretty(turn == 1) << std::endl;
        undo_buffer.push_back(s);
        if (s.lose()) {
            std::cout << "====================================" << std::endl;
            std::cout << "====================================" << std::endl;
            std::cout << "Player " << 1 - turn << " wins!!" << std::endl;
            while (true) {
                std::string line;
                std::getline(out[turn], line);
                std::cout << line << std::endl;

                if (line == "UNDO") {
                    if (undo_buffer.size() >= 2) {
                        undo_buffer.resize(undo_buffer.size() - 2);
                        s = undo_buffer.back();
                    }
                    goto back;
                }
            }
        }
        //break;

        in[turn] << s << std::endl;
        std::string line;
        std::getline(out[turn], line);
        std::cout << line << std::endl;

        if (line == "UNDO") {
            if (undo_buffer.size() >= 2) {
                undo_buffer.resize(undo_buffer.size() - 2);
                s = undo_buffer.back();
            }
            continue;
        }

        std::istringstream ss{line};
        ss.exceptions(std::ios_base::failbit);
        Quoridor::State::NextMove m;

        try {
            ss >> m;
        } catch (std::exception& ex) {
            std::cout << "Invalid command: " << ex.what() << std::endl;
            break;
        }


        auto ns = s.move(m);
        if (!ns.has_value()) {
            std::cout << "Invalid move: " << m << std::endl;
            break;
        } else {
            turn = 1 - turn;
            s = *ns;
        }
    }
    std::cout << "====================================" << std::endl;
    std::cout << "====================================" << std::endl;
    std::cout << "Player " << 1 - turn << " wins!!" << std::endl;


    return 0;
}