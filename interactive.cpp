#include "game.hpp"
#include "negaalpha.hpp"
#include <algorithm>
#include <chrono>
#include <iostream>
int main()
{

    Game2048::AttackerStateMulti state = {{}, 1};
    while (true) {
        std::cout << state << std::endl;
        Entry<Game2048::AttackerStateMulti> res;
        using namespace std::chrono_literals;
        auto start_time = std::chrono::steady_clock::now();
        int depth = 2;
        for (; depth <= 20; depth += 2) {
            try {
                res = iterativeDeepeningNegaalpha(state, Game2048::eval, depth, depth - 2, [&] { return std::chrono::steady_clock::now() - start_time > 900ms; });
            } catch (Abort&) {
                break;
            }
        }
        depth -= 2;

        auto& [score, move, user_state] = res;
        std::cout << depth << " " << score << " " << move << std::endl;
        std::cout << user_state;


        std::vector<Game2048::ReducerMove> next_entries;
        next_entries.reserve(1024);
        forEachMove(user_state, [&](const Game2048::ReducerMove& move, const Game2048::AttackerState& next_state) {
            next_entries.push_back(move);
        });
        int best_path_id = best_path_table[0][1];
        if (best_path_id != -1 && next_entries.size() > 0) {
            std::cout << "Hint: " << next_entries[best_path_id] << std::endl;
        }

        while (true) {
            Game2048::ReducerMove m;
            while (true) {
                char c;
                std::cin >> c;
                constexpr char commands[] = "lkhj";
                int offset = std::find(commands, commands + 4, c) - commands;
                if (offset >= 0 && offset <= 3) {
                    m = {offset};
                    break;
                }
            }
            std::cout << std::endl;
            std::cout << "Move input: " << m << std::endl;
            auto next = user_state.move(m);
            if (!next) {
                std::cout << "Invalid move. Try again" << std::endl;
            } else {
                state = {{*next}, 1};
                break;
            }
        }
    }
    return 0;
}