#pragma once
#pragma once
#include "negaalpha.hpp"
#include <array>
#include <cmath>
#include <iostream>
#include <optional>
#include <string>
#include <vector>
namespace Game2048Full
{


struct Point {
    int x, y;
};
template <int turn>
struct Move {
    int dir;
    int n;
    std::vector<Point> points;

    friend std::ostream& operator<<(std::ostream& os, const Move& m)
    {
        os << "<" << m.dir << "> & ";
        os << "<" << m.n << " @";
        for (auto& p : m.points) {
            os << " <" << p.x << " " << p.y << ">";
        }
        os << ">";
        return os;
    }
};

template <int turn, bool first = false>
struct State {
    using NextMove = Move<turn>;
    using NextState = State<turn ^ 1>;
    //    std::array<int, std::array<std::array<int, 5>, 5>, 2> data;
    std::array<std::array<int, 5>, 5> self;
    std::array<std::array<int, 5>, 5> opponent;
    friend std::ostream& operator<<(std::ostream& os, const State& s)
    {
        for (int y = 0; y < 5; y++) {
            for (int x = 0; x < 5; x++) {
                os << (s.self[y][x] == 0 ? std::string(".") : std::to_string(s.self[y][x])) << ' ';
            }

            os << '\n';
        }
        os << '\n';

        for (int y = 0; y < 5; y++) {
            for (int x = 0; x < 5; x++) {
                os << (s.opponent[y][x] == 0 ? std::string(".") : std::to_string(s.opponent[y][x])) << ' ';
            }

            os << '\n';
        }
        os << '\n';
        return os;
    }

    std::pair<int, std::array<std::array<int, 5>, 5>> moveReduce(int dir) const;
    std::optional<std::array<std::array<int, 5>, 5>> moveAttack(int n, const std::vector<Point>& points) const;
    std::optional<State<turn ^ 1>> move(const Move<turn>& m) const
    {
        auto [reduction, next_opponent] = moveReduce(m.dir);
        if (reduction == -1)
            return std::nullopt;
        if ((1 << (reduction - m.n + 1)) != m.points.size())
            return std::nullopt;
        auto next_self_opt = moveAttack(m.n, m.points);
        if (!next_self_opt)
            return std::nullopt;
        std::optional<State<turn ^ 1>> ret;
        return {*next_self_opt, opponent};
    }
};
template <int turn, bool first>
std::pair<int, std::array<std::array<int, 5>, 5>> State<turn, first>::moveReduce(int dir) const
{

    auto slide = [&](const auto& access) {
        std::array<std::array<int, 5>, 5> s = {};
        bool moved = false;
        int reduction = 0;
        for (int y = 0; y < 5; y++) {
            int ix = 0;
            int prev = 0;
            for (int x = 0; x < 5; x++) {
                int num = access(self, x, y);
                if (num != 0) {
                    if (prev == num) {
                        access(s, ix - 1, y)++;
                        prev = 0;
                        moved = true;
                        reduction++;
                    } else {
                        access(s, ix, y) = num;
                        prev = num;
                        moved |= (ix != x);
                        ix++;
                    }
                }
            }
        }
        if (!moved) {
            return std::make_pair(-1, s);
        }
        return std::make_pair(reduction, s);
    };

    switch (dir) {
    case 0:
        return slide([&](auto& arr, int x, int y) -> auto& { return arr[y][4 - x]; });
    case 1:
        return slide([&](auto& arr, int x, int y) -> auto& { return arr[x][y]; });
    case 2:
        return slide([&](auto& arr, int x, int y) -> auto& { return arr[y][x]; });
    case 3:
        return slide([&](auto& arr, int x, int y) -> auto& { return arr[4 - x][y]; });
    default:
        return {-1, {}};
    }
}

template <int turn, bool first>
std::optional<std::array<std::array<int, 5>, 5>> State<turn, first>::moveAttack(int n, const std::vector<Point>& points) const
{
    std::array<std::array<int, 5>, 5> out = opponent;
    for (auto& p : points) {
        if (out[p.y][p.x] != 0)
            return std::nullopt;
        out[p.y][p.x] = n;
    }
    return out;
}


template <int turn, bool first, class F>
void forEachMove(const State<turn, first>& state, const F& func)
{

    std::vector<Point> valid_points;
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            if (state.opponent[y][x] == 0)
                valid_points.push_back({x, y});
        }
    }

    constexpr int threshold = 5;
    for (int i = 0; i <= 3; i++) {
        int dir = i;
        if (auto [reduction, next_opponent] = state.moveReduce(dir); reduction != -1) {
            if (first || valid_points.size() <= threshold) {
                auto put = [&](int n) {
                    int num = 1 << (reduction + 1 - n);
                    using bitset_type = std::uint32_t;
                    auto subset_combination = [](int n, int k, const auto& func2) {
                        bitset_type S = bitset_type((bitset_type(1) << k) - 1);
                        bitset_type E = ~bitset_type((bitset_type(1) << n) - 1);
                        while (!(S & E)) {
                            func2(S, n);
                            bitset_type smallest = S & -S;
                            bitset_type ripple = S + smallest;
                            bitset_type nsmallest = ripple & -ripple;
                            int trailing_zeros = __builtin_ctzll(smallest);
                            S = ripple | ((nsmallest >> (trailing_zeros + 1)) - 1);
                        }
                    };
                    if (int(valid_points.size()) < num)
                        return;
                    subset_combination(valid_points.size(), num, [&](bitset_type S, int) {
                        Move<turn> m;
                        m.n = n;
                        m.points.reserve(num);
                        bitset_type bit;
                        for (bitset_type rem = S; rem != 0; rem &= ~bit) {
                            bit = rem & -rem;
                            int i = __builtin_ctz(bit);
                            m.points.push_back(valid_points[i]);
                        }
                        auto next_self = state.moveAttack(n, m.points);
                        m.dir = dir;
                        func(m, typename State<turn, first>::NextState{*next_self, next_opponent});
                    });
                };
                for (int n = 1; n <= reduction + 1; n++) {
                    put(n);
                }
            } else {
                Move<turn> m;
                m.dir = dir;
                m.points.resize(1);
                for (auto& p : valid_points) {
                    m.points[0] = p;
                    for (int n = 1; n <= 2; n++) {
                        m.n = n;
                        auto next_self = state.moveAttack(n, m.points);
                        func(m, typename State<turn, first>::NextState{*next_self, next_opponent});
                    }
                }
            }
        }
    }
}
//using TScore = int;
//constexpr TScore INF = 2000000000;  //2147483647;
using ll = long long;
int pow(int x)
{
    return x * x * x;
}
inline static auto eval = [](const auto& s) {
    int opponent_num = 0;
    //    for (int y = 0; y < 5; y++) {
    //        for (int x = 0; x < 5; x++) {
    //            if (s.opponent[y][x] != 0)
    //                opponent_num += s.opponent[y][x] + 1;
    //        }
    //    }

    opponent_num = continuous(s.opponent);

    int self_num_min = INF;

    /*auto s2=s;
    for(int y=0;y<5;y++){
        for(int x=0;x<5;x++){
            if(s2.self[y][x]==0)s2.self[y][x]=1;
        }
    }
    for (int dir = 0; dir < 3; dir++) {
        auto [reduction, state] = s2.moveReduce(dir);
        if (reduction==-1)
            continue;
        int self_num = 0;
        for (int y = 0; y < 5; y++) {
            for (int x = 0; x < 5; x++) {
                if (state[y][x] != 0)
                    self_num += (state[y][x]) + 5;
            }
        }
        self_num_min = std::min(self_num, self_num_min);
    }*/

    //    std::cerr << s << "\n";
    for (int dir = 0; dir <= 3; dir++) {
        auto [reduction, state] = s.moveReduce(dir);
        //        std::cerr << dir <<" "<< reduction << std::endl;
        if (reduction == -1)
            continue;
        //        int self_num = 0;
        //        for (int y = 0; y < 5; y++) {
        //            for (int x = 0; x < 5; x++) {
        //                if (state[y][x] != 0)
        //                    self_num += (state[y][x]) + 5;
        //            }
        //        }
        self_num_min = std::min(continuous(state), self_num_min);
    }
    if (self_num_min == INF)
        return -INF;

    TScore score = pow(opponent_num) - pow(self_num_min);

    return score;
};
};  // namespace Game2048Full
