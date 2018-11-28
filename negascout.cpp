#include <array>

#include "negaalpha.hpp"
#include "quoridor.hpp"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>


int main(int argc, const char** argv)
{
    std::ios_base::sync_with_stdio(false);
    //    if (argc != 3) {
    //        std::cerr << "Usage: interactive <in-path> <out-path>" << std::endl;
    //        return 1;
    //    }
    //    std::ifstream in(argv[1]);
    //    std::ofstream out(argv[2]);

    verbose_depth = 4;
    verbose_search_depth = 7;
    while (true) {
        Quoridor::State s{};
        std::cin >> s;

        if (std::cin.eof()) {
            std::cerr << "in.eof()" << std::endl;
            return 0;
        }
        std::cerr << s << "\n\n";
        std::cerr << s.pretty() << '\n';

        auto start_time = std::chrono::steady_clock::now();
        Result<Quoridor::State> res = {};
        try {
            for (int i = 1; i <= 20; i++) {
                res = iterativeDeepeningNegascout(s,
                    [&](const Quoridor::State& state) {
                        if (state.lose())
                            return -INF;
                        return -search(state, state.my_pos, [&](const Quoridor::Position& p) { return p.y == 0; })
                               + search(state, state.opponent_pos, [&](const Quoridor::Position& p) { return p.y == Quoridor::N - 1; });
                    },
                    i,
                    i - 1,
                    [&] {
#ifdef NDEBUG
                        return std::chrono::steady_clock::now() - start_time >= std::chrono::milliseconds(980);
#else
                        return false;
#endif
                    });

                std::cerr << res << " @ " << (std::chrono::steady_clock::now() - start_time).count() / 1.0e6 << "ms" << std::endl;
            }
        } catch (Abort&) {
        }


        auto m = res.getMove();
        std::cerr << m << std::endl;

        std::cout << m << std::endl;
    }
    return 0;
}