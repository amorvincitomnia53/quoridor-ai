#pragma once
#include <array>
#include <cassert>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace Game2048
{

struct Point {
    int x, y;
};
struct AttackerMove {
    int n;
    Point p;

    friend std::ostream& operator<<(std::ostream& os, const AttackerMove& m)
    {
        os << "<" << m.n << " @ " << m.p.x << " " << m.p.y << ">";
        return os;
    }
};


struct AttackerMoveMulti {
    int n;
    std::vector<Point> points;

    friend std::ostream& operator<<(std::ostream& os, const AttackerMoveMulti& m)
    {
        os << "<" << m.n << " @";
        for (auto& p : m.points) {
            os << " (" << p.x << " " << p.y << ")";
        }
        os << ">";
        return os;
    }
};

struct ReducerMove {
    int dir;

    friend std::ostream& operator<<(std::ostream& os, const ReducerMove& m)
    {
        os << "<" << m.dir << ">";
        return os;
    }
};
struct StateBase {
    std::array<std::array<int, 5>, 5> data;
    friend std::ostream& operator<<(std::ostream& os, const StateBase& s)
    {
        for (int y = 0; y < 5; y++) {
            for (int x = 0; x < 5; x++) {
                os << (s.data[y][x] == 0 ? std::string(".") : std::to_string(s.data[y][x])) << ' ';
            }

            os << '\n';
        }
        return os;
    }
    friend std::istream& operator>>(std::istream& is, StateBase& s)
    {
        for (int y = 0; y < 5; y++) {
            for (int x = 0; x < 5; x++) {
                std::cin >> s.data[y][x];
            }
        }
        return is;
    }
};
struct AttackerState;
struct ReducerState : public StateBase {
    using NextMove = ReducerMove;
    using NextState = AttackerState;
    static constexpr bool reducer_turn = true;

    std::optional<AttackerState> move(const ReducerMove&) const;

    int countReduction(const ReducerMove&) const;
};

struct AttackerState : public StateBase {
    using NextMove = AttackerMove;
    using NextState = ReducerState;
    static constexpr bool reducer_turn = false;


    std::optional<ReducerState> move(const AttackerMove&) const;
};

struct AttackerStateMulti : public StateBase {
    using NextMove = AttackerMoveMulti;
    using NextState = ReducerState;
    static constexpr bool reducer_turn = false;

    int n = 1;
    std::optional<ReducerState> move(const AttackerMoveMulti&) const;
};

std::optional<AttackerState> ReducerState::move(const ReducerMove& m) const
{

    auto slide = [&](const auto& access) -> std::optional<AttackerState> {
        AttackerState s = {};
        bool moved = false;
        for (int y = 0; y < 5; y++) {
            int ix = 0;
            int prev = 0;
            for (int x = 0; x < 5; x++) {
                int num = access(data, x, y);
                if (num != 0) {
                    if (prev == num) {
                        access(s.data, ix - 1, y)++;
                        prev = 0;
                        moved = true;
                    } else {
                        access(s.data, ix, y) = num;
                        prev = num;
                        moved |= (ix != x);
                        ix++;
                    }
                }
            }
        }
        if (moved) {
            return s;
        } else
            return std::nullopt;
    };

    switch (m.dir) {
    case 0:
        return slide([&](auto& arr, int x, int y) -> auto& { return arr[y][4 - x]; });
    case 1:
        return slide([&](auto& arr, int x, int y) -> auto& { return arr[x][y]; });
    case 2:
        return slide([&](auto& arr, int x, int y) -> auto& { return arr[y][x]; });
    case 3:
        return slide([&](auto& arr, int x, int y) -> auto& { return arr[4 - x][y]; });
    default:
        return std::nullopt;
    }
}

int ReducerState::countReduction(const Game2048::ReducerMove& m) const
{

    auto slide = [&](const auto& access) {
        int reduction = 0;
        for (int y = 0; y < 5; y++) {
            int prev = 0;
            for (int x = 0; x < 5; x++) {
                int num = access(data, x, y);
                if (num != 0) {
                    if (prev == num) {
                        prev = 0;
                        reduction++;
                    } else {
                        prev = num;
                    }
                }
            }
        }
        return reduction;
    };

    switch (m.dir) {
    case 0:
        return slide([&](auto& arr, int x, int y) -> auto& { return arr[y][4 - x]; });
    case 1:
        return slide([&](auto& arr, int x, int y) -> auto& { return arr[x][y]; });
    case 2:
        return slide([&](auto& arr, int x, int y) -> auto& { return arr[y][x]; });
    case 3:
        return slide([&](auto& arr, int x, int y) -> auto& { return arr[4 - x][y]; });
    default:
        return -1;
    }
}

std::optional<ReducerState> AttackerState::move(const AttackerMove& m) const
{
    if (data[m.p.y][m.p.x] == 0) {
        ReducerState s{{*this}};
        s.data[m.p.y][m.p.x] = m.n;
        return s;
    } else {
        return std::nullopt;
    }
}

std::optional<ReducerState> AttackerStateMulti::move(const AttackerMoveMulti& m) const
{
    ReducerState ret = {{*this}};
    int num_log = __builtin_ctzl(m.points.size());
    if ((1u << num_log) != m.points.size())
        return std::nullopt;

    if (m.n != n - num_log)
        return std::nullopt;
    for (auto& p : m.points) {
        if (ret.data[p.y][p.x] != 0)
            return std::nullopt;
        ret.data[p.y][p.x] = m.n;
    }
    return ret;
}


template <class F>
void forEachMove(const ReducerState& state, const F& func)
{
    for (int i = 0; i <= 3; i++) {
        auto m = ReducerMove{i};
        if (auto next = state.move(m); next) {
            func(m, *next);
        }
    }
}

template <class F>
void forEachMove(const AttackerState& state, const F& func)
{
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            for (int n = 1; n <= 3; n++) {
                auto m = AttackerMove{n, x, y};
                if (auto next = state.move(m); next) {
                    func(m, *next);
                }
            }
        }
    }
}


template <class F>
void forEachMove(const AttackerStateMulti& state, const F& func)
{
    std::vector<Point> valid_points;
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            if (state.data[y][x] == 0)
                valid_points.push_back({x, y});
        }
    }

    auto put = [&](int n) {
        int num = 1 << (state.n - n);
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
            AttackerMoveMulti m;
            m.n = n;
            m.points.reserve(num);
            bitset_type bit;
            for (bitset_type rem = S; rem != 0; rem &= ~bit) {
                bit = rem & -rem;
                int i = __builtin_ctz(bit);
                m.points.push_back(valid_points[i]);
            }
            assert(m.points.size() != 0);
            func(m, *state.move(m));
        });
    };
    for (int n = 1; n <= state.n; n++) {
        put(n);
    }
}
using TScore = int;
inline static auto eval = [](const auto& s) {
    //    int num = 0;


    /*ReducerState s2{{s}};
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            if (s2.data[y][x] == 0)
                s2.data[y][x] = 1;
        }
    }
    int min_num = std::numeric_limits<int>::max();
    for (int dir = 0; dir < 3; dir++) {
        auto state = s2.move({dir});
        if(!state)continue;

        for (int y = 0; y < 5; y++) {
            for (int x = 0; x < 5; x++) {
                if (state->data[y][x] != 0)
                    num += (state->data[y][x]) + 2;
            }
        }
        min_num=std::min(min_num, num);
    }
    TScore reducer_score = -num;
    */
    /*
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            if (s.data[y][x] != 0)
                num += (s.data[y][x]) + 2;
        }
    }

*/
    //    return -attacker_score;

    return (s.reducer_turn ? -1 : 1) * continuous(s.data);
};
};  // namespace Game2048
