#include <array>

#include "negaalpha.hpp"
#include "quoridor_fast.hpp"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>


int main(int argc, const char** argv)
{
    std::ios_base::sync_with_stdio(false);

    verbose_depth = 4;
    verbose_search_depth = 7;
    while (true) {
        QuoridorFast::State s{};
        std::cin >> s;

        if (std::cin.eof()) {
            std::cerr << "in.eof()" << std::endl;
            return 0;
        }
        std::cerr << s << "\n\n";
        std::cerr << s.pretty() << '\n';

        auto start_time = std::chrono::steady_clock::now();
        Result<QuoridorFast::State> res = {};
        try {
            for (int i = 1; i <=
#ifdef NDEBUG
                            20
#else
                            2
#endif
                 ;
                 i++) {
                res = iterativeDeepeningNegascout(s,
                    [](const QuoridorFast::State& state) {
                        if (state.lose())
                            return -INF;
                        auto hfullwall = state.hFullWall();
                        auto vfullwall = state.vFullWall();
                        int myps = QuoridorFast::potentialSearch(hfullwall, vfullwall, QuoridorFast::my_goal, QuoridorFast::BitBoard::oneHot(state.my_pos));
                        int oppps = QuoridorFast::potentialSearch(hfullwall, vfullwall, QuoridorFast::BitBoard::oneHot(state.opponent_pos), QuoridorFast::opponent_goal);
                        if (myps == -1 || oppps == -1)
                            return INF;
                        int my_dist = myps + state.my_pos.y;
                        int opp_dist = oppps + QuoridorFast::N - 1 - state.opponent_pos.y;
                        //                        std::cerr << state.pretty() << std::endl
                        //                                  << my_dist << " " << opp_dist << " " << opp_dist - my_dist << std::endl;
                        //auto wall_func = [](int x) { return int(std::log(x + 0.3) * 8 * 100000); };
                        auto wall_func = [](int self, int opponent) {
                            return int(std::tanh(opponent * 0.3) * (1.2 * self - 4.0 / (self + 0.3)) * 100000);
                        };
                        /*if(s.my_rem_walls != s.opponent_rem_walls)
                            std::cerr<<s.my_rem_walls << s.opponent_rem_walls
                            <<" " <<cube(s.my_rem_walls - s.opponent_rem_walls)* 100023
                            << " " << ((opp_dist - my_dist) * 100000 + cube(s.my_rem_walls - s.opponent_rem_walls) * 100023)<<std::endl;
                        */


                        return ((opp_dist - my_dist) * 100000 + wall_func(state.my_rem_walls, state.opponent_rem_walls) - wall_func(state.opponent_rem_walls, state.my_rem_walls));  // * 4 + s.my_rem_walls * 1 + cube(s.my_rem_walls - s.opponent_rem_walls);
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
