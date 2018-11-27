#pragma once
#include <array>
#include <iostream>
#include <optional>
#include <queue>
#include <type_traits>
#include <variant>

namespace Quoridor
{

constexpr int N = 9;

enum class WallDirection {
    Horizontal = 0,
    Vertical = 1
};


enum class WallState {
    NoWall = 0,
    Horizontal = 1,
    Vertical = 2
};


enum class MoveDirection {
    Right = 0,
    Up = 1,
    Left = 2,
    Down = 3
};


struct Position {
    int x, y;
    friend std::ostream& operator<<(std::ostream& os, const Position& p)
    {
        return os << p.x << " " << p.y;
    }
    friend std::istream& operator>>(std::istream& is, Position& p)
    {
        return is >> p.x >> p.y;
    }
    constexpr bool operator==(const Position& p2) const
    {
        return x == p2.x && y == p2.y;
    }
    constexpr bool operator!=(const Position& p2) const
    {
        return !(*this == p2);
    }
    constexpr Position operator+(const Position& p2) const
    {
        return {x + p2.x, y + p2.y};
    }

    constexpr Position operator-(const Position& p2) const
    {
        return {x - p2.x, y - p2.y};
    }

    constexpr Position& operator+=(const Position& p2)
    {
        x += p2.x;
        y += p2.y;
        return *this;
    }

    constexpr Position& operator-=(const Position& p2)
    {
        x -= p2.x;
        y -= p2.y;
        return *this;
    }
};
inline Position rotate(Position pos, int n90)
{
    switch (n90 & 3) {
    case 0:
        return pos;
    case 1:
        return {pos.y, -pos.x};
    case 2:
        return {-pos.x, -pos.y};
    case 3:
        return {-pos.y, pos.x};
    default:
        return {};
    }
}

inline constexpr bool inRange(const Position& p, int limit = N) { return p.x >= 0 && p.y >= 0 && p.x < limit && p.y < limit; }
struct Advance {
    Position new_position;

    friend std::ostream& operator<<(std::ostream& os, const Advance& m)
    {
        return os << "MOVE " << m.new_position;
    }

    friend std::istream& operator>>(std::istream& is, Advance& m)
    {
        return is >> m.new_position;
    }
};

struct Put {
    WallDirection direction;
    Position pos;


    friend std::ostream& operator<<(std::ostream& os, const Put& m)
    {
        return os << "WALL "
                  << "HV"[int(m.direction)] << " " << m.pos;
    }

    friend std::istream& operator>>(std::istream& is, Put& m)
    {
        char dir;
        is >> dir >> m.pos;
        m.direction = (dir == 'H' ? WallDirection::Horizontal : dir == 'V' ? WallDirection::Vertical : throw std::invalid_argument("dir"));
        return is;
    }
};


std::ostream& operator<<(std::ostream& os, const std::variant<Advance, Put>& m)
{
    std::visit([&](const auto& m2) mutable { os << m2; }, m);
    return os;
}

std::istream& operator>>(std::istream& is, std::variant<Advance, Put>& m)
{
    std::string command;
    is >> command;
    if (command == "MOVE") {
        Advance adv{};
        is >> adv;
        m = adv;
    } else if (command == "WALL") {
        Put put{};
        is >> put;
        m = put;
    } else {
        throw std::invalid_argument("command");
    }
    return is;
}

template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...)->overloaded<Ts...>;

std::array<std::array<WallState, N - 1>, N - 1> flipWalls(const std::array<std::array<WallState, N - 1>, N - 1>& walls)
{
    std::array<std::array<WallState, N - 1>, N - 1> ret{};
    for (int y = 0; y < N - 1; y++) {
        for (int x = 0; x < N - 1; x++) {
            ret[y][x] = walls[N - 2 - y][N - 2 - x];
        }
    }
    return ret;
}


struct State {


    //! この2つの型別名定義がnegascoutに必要
    using NextMove = std::variant<Advance, Put>;  //!< 次の手の型
    using NextState = State;                      //!< 次の状態の型。


    Position my_pos;
    int my_rem_walls;

    Position opponent_pos;
    int opponent_rem_walls;

    std::array<std::array<WallState, N - 1>, N - 1> m_walls;


    bool operator==(const State& s2)
    {
        return my_pos == s2.my_pos
               && opponent_pos == s2.opponent_pos
               && my_rem_walls == s2.my_rem_walls
               && opponent_rem_walls == s2.opponent_rem_walls
               && m_walls == s2.m_walls;
    }
    bool operator!=(const State& s2)
    {
        return !(*this == s2);
    }
    static State initialState()
    {
        return State{{4, 8}, 10, {4, 0}, 10, {}};
    }

    WallState walls(Position p) const
    {
        if (p.x < 0 || p.x >= N - 1)
            return WallState::Vertical;
        else if (p.y < 0 || p.y >= N - 1)
            return WallState::Horizontal;
        else
            return m_walls[p.y][p.x];
    }
    bool hasWall(Position p, MoveDirection dir) const
    {
        switch (dir) {
        case MoveDirection::Right:
            return walls(p) == WallState::Vertical || walls(p - Position{0, 1}) == WallState::Vertical;
        case MoveDirection ::Up:
            return walls(p - Position{0, 1}) == WallState::Horizontal || walls(p - Position{1, 1}) == WallState::Horizontal;
        case MoveDirection ::Left:
            return walls(p - Position{1, 0}) == WallState::Vertical || walls(p - Position{1, 1}) == WallState::Vertical;
        case MoveDirection ::Down:
            return walls(p) == WallState::Horizontal || walls(p - Position{1, 0}) == WallState::Horizontal;
        default:
            return false;
        }
    }

    State flipPlayers() const
    {
        State ret{};
        ret.my_pos = Position{N - 1, N - 1} - opponent_pos;
        ret.opponent_pos = Position{N - 1, N - 1} - my_pos;
        ret.my_rem_walls = opponent_rem_walls;
        ret.opponent_rem_walls = my_rem_walls;
        ret.m_walls = flipWalls(m_walls);
        return ret;
    }
    bool lose() const
    {
        return opponent_pos.y == N - 1;
    }
    template <class F>
    friend int search(const State& state, const Position& start, const F& goal)
    {

        std::array<std::array<bool, N>, N> visited = {};

        struct Entry {
            Position p;
            int dist;
        };
        std::queue<Entry> Q;
        Q.push({start, 0});
        while (!Q.empty()) {
            auto [p, dist] = Q.front();
            Q.pop();

            if (visited[p.y][p.x])
                continue;
            visited[p.y][p.x] = true;
            if (goal(p)) {
                return dist;
            }
            if (!state.hasWall(p, MoveDirection::Right) && inRange(p + Position{1, 0})) {
                Q.push({p + Position{1, 0}, dist + 1});
            }
            if (!state.hasWall(p, MoveDirection::Up) && inRange(p + Position{0, -1})) {
                Q.push({p + Position{0, -1}, dist + 1});
            }
            if (!state.hasWall(p, MoveDirection::Left) && inRange(p + Position{-1, 0})) {
                Q.push({p + Position{-1, 0}, dist + 1});
            }
            if (!state.hasWall(p, MoveDirection::Down) && inRange(p + Position{0, 1})) {
                Q.push({p + Position{0, 1}, dist + 1});
            }
        }
        return -1;
    }
    /*!
     * この状態から指定された手をうつ。その手が有効かどうかの判定も行う。
     * @param m 手
     * @return その手が有効であれば次の状態、無効であればstd::nulloptを返す。
     */
    std::optional<NextState> move(const NextMove& m) const
    {
        return std::visit(overloaded{
                              [&](const Advance& m) -> std::optional<NextState> {
                                  if (!inRange(m.new_position) || m.new_position == opponent_pos)
                                      return std::nullopt;
                                  Position diff = m.new_position - my_pos;
                                  for (int i = 0; i < 4; i++) {
                                      Position rotd = rotate(diff, -i);
                                      if ((rotd == Position{1, 0}
                                              && !hasWall(my_pos, (MoveDirection)i))
                                          || (opponent_pos == my_pos + rotate({1, 0}, i)
                                                 && !hasWall(my_pos, (MoveDirection)i)
                                                 && ((!hasWall(opponent_pos, (MoveDirection)i) && rotd == Position{2, 0})
                                                        || (hasWall(opponent_pos, (MoveDirection)i)
                                                               && ((rotd == Position{1, 1} && !hasWall(opponent_pos, (MoveDirection)((i - 1) & 3)))
                                                                      || (rotd == Position{1, -1} && !hasWall(opponent_pos, (MoveDirection)((i + 1) & 3)))))))) {
                                          State flipped = flipPlayers();
                                          flipped.opponent_pos = Position{N - 1, N - 1} - m.new_position;
                                          return flipped;
                                      }
                                  }
                                  return std::nullopt;
                              },
                              [&](const Put& m) -> std::optional<NextState> {
                                  if (my_rem_walls == 0                     // 残り壁枚数チェック
                                      || !inRange(m.pos, N - 1)             // 範囲チェック
                                      || walls(m.pos) != WallState::NoWall  // 重複チェック
                                      || (m.direction == WallDirection::Horizontal ? walls(m.pos - Position{1, 0}) == WallState::Horizontal
                                                                                   : walls(m.pos - Position{0, 1}) == WallState::Vertical)
                                      || (m.direction == WallDirection::Horizontal ? walls(m.pos + Position{1, 0}) == WallState::Horizontal
                                                                                   : walls(m.pos + Position{0, 1}) == WallState::Vertical)) {  // 平行干渉チェック
                                      return std::nullopt;
                                  }


                                  State flipped = flipPlayers();
                                  flipped.m_walls[N - 2 - m.pos.y][N - 2 - m.pos.x] = m.direction == WallDirection::Horizontal ? WallState::Horizontal : WallState::Vertical;
                                  // 自分や相手が動けなくなるような置き方をしてはいけない
                                  if (search(flipped, flipped.my_pos, [&](const auto& p) { return p.y == 0; }) == -1) {
                                      return std::nullopt;
                                  }
                                  if (search(flipped, flipped.opponent_pos, [&](const auto& p) { return p.y == N - 1; }) == -1) {
                                      return std::nullopt;
                                  }

                                  flipped.opponent_rem_walls--;
                                  return flipped;
                              }},
            m);
    }


    /*!
     * 動かせる全ての手を列挙し、関数オブジェクトfuncを呼ぶ。
     * @param state 現在の状態
     * @param func 呼ぶ関数オブジェクト
     */
    template <class F>
    void forEachMove(const F& func) const
    {
        {
            for (int i = 0; i < 4; i++) {
                if (hasWall(my_pos, (MoveDirection)i))
                    continue;
                auto call = [&](const Position& newp) {
                    NextMove m = Advance{newp};
                    State flipped = flipPlayers();
                    flipped.opponent_pos = Position{N - 1, N - 1} - newp;
                    func(m, flipped);
                };
                if (opponent_pos != my_pos + rotate({1, 0}, i)) {
                    call(my_pos + rotate({1, 0}, i));
                } else if (!hasWall(opponent_pos, (MoveDirection)i)) {
                    call(my_pos + rotate({2, 0}, i));
                } else {
                    if (!hasWall(opponent_pos, (MoveDirection)((i - 1) & 3))) {
                        call(my_pos + rotate({1, 1}, i));
                    }
                    if (!hasWall(opponent_pos, (MoveDirection)((i + 1) & 3))) {
                        call(my_pos + rotate({1, -1}, i));
                    }
                }
            }
        }
        {
            for (int y = 0; y < N - 1; y++) {
                for (int x = 0; x < N - 1; x++) {
                    auto call = [&](WallDirection dir) {
                        NextMove m = Put{dir, {x, y}};
                        std::optional<State> ns = move(m);
                        if (ns) {
                            func(m, *ns);
                        }
                    };
                    call(WallDirection::Horizontal);
                    call(WallDirection::Vertical);
                }
            }
        }
    }


    struct PrettyFormat;
    PrettyFormat pretty(bool flip = false) const;
};

struct State::PrettyFormat : State {
    bool flip;
};

State::PrettyFormat State::pretty(bool flip) const
{
    return {{*this}, flip};
}

/*!
 * わかりやすい形で出力する。
 */
std::ostream& operator<<(std::ostream& os, const State::PrettyFormat& s)
{
    os << '|' << std::string(2 * N - 1, '-') << "|\n";
    State s2 = s.flip ? s.flipPlayers() : static_cast<State>(s);
    for (int y = 0; y < N; y++) {

        os << '|';
        for (int x = 0; x < N; x++) {
            Position pos{x, y};
            bool movable = s.move(Advance{s.flip ? Position{N - 1, N - 1} - pos : pos}).has_value();
            os << (s2.my_pos == pos ? '^' : s2.opponent_pos == pos ? 'v' : movable ? 'o' : '.')
               << (s2.hasWall(pos, MoveDirection::Right) ? '|' : ' ');
        }
        os << '\n';
        os << '|';
        for (int x = 0; x < N; x++) {
            os << (s2.hasWall({x, y}, MoveDirection::Down) ? '-' : ' ')
               << (s2.walls({x, y}) == WallState::Horizontal ? '-' : s2.walls({x, y}) == WallState::Vertical ? '|' : ' ');
        }
        os << '\n';
    }
    os << "Remaining walls: {YOU: " << s.my_rem_walls << ", OPPONENT: " << s.opponent_rem_walls << "}\n";
    return os;
}


std::ostream& operator<<(std::ostream& os, const State& s)
{
    os << s.my_pos << ' ' << s.opponent_pos << ' ' << s.my_rem_walls << ' ' << s.opponent_rem_walls << '\n';
    for (int y = 0; y < N - 1; y++) {
        for (int x = 0; x < N - 1; x++) {
            os << (int)s.walls({x, y}) << (x == N - 2 ? '\n' : ' ');
        }
    }
    return os;
}

std::istream& operator>>(std::istream& is, State& s)
{
    s = {};
    is >> s.my_pos >> s.opponent_pos >> s.my_rem_walls >> s.opponent_rem_walls;
    for (int y = 0; y < N - 1; y++) {
        for (int x = 0; x < N - 1; x++) {
            int w;
            is >> w;
            s.m_walls[y][x] = (WallState)w;
        }
    }
    return is;
}


}  // namespace Quoridor
