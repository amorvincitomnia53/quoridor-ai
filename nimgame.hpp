#pragma once
#include <array>
#include <cmath>
#include <iostream>

struct Nim {

    template <int turn = 0>
    struct Move {
        using Game = Nim;
        int bin, num;

        friend std::ostream& operator<<(std::ostream& os, const Move& m)
        {
            os << "<" << m.bin << "; " << m.num << ">";
            return os;
        }
    };
    template <int turn = 0>
    struct State {
        using Game = Nim;
        using NextMove = Move<turn>;
        using NextState = State<(turn + 1) % 2>;
        std::array<int, 3> x;

        friend std::ostream& operator<<(std::ostream& os, const State& s)
        {
            os << "(" << s.x[0] << " " << s.x[1] << " " << s.x[2] << ")";
            return os;
        }

        template <class F>
        friend void forEachMove(const State& state, const F& func)
        {
            for (int i = 0; i <= 2; i++) {
                for (int j = 1; j <= state.x[i]; j++) {
                    NextState s;
                    s.x = state.x;
                    s.x[i] -= j;
                    func(NextMove{i, j}, s);
                }
            }
        }
    };

    inline static auto eval = [](const auto& s) {
        return -(s.x[0] ^ s.x[1] ^ s.x[2]);
    };
};
