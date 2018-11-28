#include "negaalpha.hpp"
#include "quoridor.hpp"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
int main(int argc, const char** argv)
{
    if (argc != 3) {
        std::cerr << "Usage: interactive <in-path> <out-path>" << std::endl;
        return 1;
    }
    std::ifstream in(argv[1]);
    std::ofstream out(argv[2]);
    while (true) {
        Quoridor::State s;
        in >> s;

        if (in.eof()) {
            std::cerr << "in.eof()" << std::endl;
            return 0;
        }
        std::cout << s.pretty() << std::endl;
        while (true) {
            std::string line;
            std::getline(std::cin, line);
            if (line == "UNDO") {
                out << "UNDO" << std::endl;
                continue;
            }
            std::istringstream ss{line};
            ss.exceptions(std::ios_base::failbit);
            Quoridor::State::NextMove m;
            try {
                ss >> m;
            } catch (std::exception& ex) {
                std::cerr << "Invalid command: " << ex.what() << std::endl;
                continue;
            }
            auto ns = s.move(m);
            if (!ns.has_value()) {
                std::cerr << "Invalid move: " << m << std::endl;
            } else {
                std::cout << m << std::endl;
                out << m << std::endl;
                break;
            }
        }
    }
    return 0;
}