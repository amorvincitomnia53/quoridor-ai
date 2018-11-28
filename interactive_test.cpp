#include "negaalpha.hpp"


#include "quoridor_fast.hpp"
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
        QuoridorFast::State s;
        in >> s;

        if (in.eof()) {
            std::cerr << "in.eof()" << std::endl;
            return 0;
        }
        //        QuoridorFast::print = [&](const QuoridorFast::BitBoard& bb) {
        //            s.printWith(std::cout, bb);
        //        };
        std::cout << s.pretty() << std::endl;
        //        s.forEachMove([&](const QuoridorFast::State::NextMove& move, const QuoridorFast::State& state) {
        //            int myps = QuoridorFast::potentialSearch(state.hFullWall(), state.vFullWall(), QuoridorFast::my_goal, QuoridorFast::BitBoard::oneHot(state.my_pos));
        //            int oppps = QuoridorFast::potentialSearch(state.hFullWall(), state.vFullWall(), QuoridorFast::BitBoard::oneHot(state.opponent_pos), QuoridorFast::opponent_goal);
        //            std::cout << move << " " << myps + state.my_pos.y << " " << oppps + QuoridorFast::N - 1 - state.opponent_pos.y << std::endl;
        //        });
        int myps = QuoridorFast::potentialSearch(s.hFullWall(), s.vFullWall(), QuoridorFast::my_goal, QuoridorFast::BitBoard::oneHot(s.my_pos));
        int oppps = QuoridorFast::potentialSearch(s.hFullWall(), s.vFullWall(), QuoridorFast::BitBoard::oneHot(s.opponent_pos), QuoridorFast::opponent_goal);
        std::cout << myps + s.my_pos.y << " " << oppps + QuoridorFast::N - 1 - s.opponent_pos.y << std::endl;
        while (true) {
            std::string line;
            std::getline(std::cin, line);
            if (line == "UNDO") {
                out << "UNDO" << std::endl;
                continue;
            }
            std::istringstream ss{line};
            ss.exceptions(std::ios_base::failbit);
            QuoridorFast::State::NextMove m;
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