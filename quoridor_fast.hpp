#pragma once

//@formatter:off

#include <array>
#include <bitset>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>
#include <optional>
#include <queue>
#include <type_traits>
#include <variant>
namespace QuoridorFast
{

constexpr int N = 9;

enum class WallDirection {
    Horizontal = 0,
    Vertical = 1
};


enum class MoveDirection {
    Right = 0,
    Up = 1,
    Left = 2,
    Down = 3
};


struct Position {
    std::int8_t x, y;
    friend std::ostream& operator<<(std::ostream& os, Position p)
    {
        return os << +p.x << " " << +p.y;
    }
    friend std::istream& operator>>(std::istream& is, Position& p)
    {
        int x, y;
        is >> x >> y;
        p.x = int8_t(x);
        p.y = int8_t(y);
        return is;
    }
    constexpr bool operator==(Position p2) const
    {
        return x == p2.x && y == p2.y;
    }
    constexpr bool operator!=(Position p2) const
    {
        return !(*this == p2);
    }
    constexpr Position operator+(Position p2) const
    {
        return {int8_t(x + p2.x), int8_t(y + p2.y)};
    }

    constexpr Position operator-(Position p2) const
    {
        return {int8_t(x - p2.x), int8_t(y - p2.y)};
    }

    constexpr Position& operator+=(Position p2)
    {
        x += p2.x;
        y += p2.y;
        return *this;
    }

    constexpr Position& operator-=(Position p2)
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
        return {pos.y, int8_t(-pos.x)};
    case 2:
        return {int8_t(-pos.x), int8_t(-pos.y)};
    case 3:
        return {int8_t(-pos.y), pos.x};
    default:
        return {};
    }
}

inline constexpr bool inRange(Position p, int limit = N) { return p.x >= 0 && p.y >= 0 && p.x < limit && p.y < limit; }
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


template <class T>
constexpr bool getBit(T x, int bit)
{
    return bool((x >> bit) & T(1));
}

template <class T>
constexpr void setBit(T& x, int bit, bool val)
{
    x = (x & ~(T(1) << bit)) | ((val ? T(1) : T(0)) << bit);
}


struct BitBoard {

    static constexpr int BIT_PER_COLUMN = 10;
    static constexpr int BIT_OFFSET = 25;

    std::array<uint64_t, 2> data;  // 1列10bit, 5列+6列


    /*
         *  14  24  34  44  54 !64  74  84  94  104 114
         *                     !
         *                     !
         *  -'- 0'- 1'- 2'- 3'-!4'- 5'- 6'- 7'- 8'- 9'-
         *      -----------------------------------
         *  -'0|0'0 1'0 2'0 3'0!4'0 5'0 6'0 7'0 8'0|9'0
         *     |               !                   |
         *  -'1|0'1 1'1 2'1 3'1!4'1 5'1 6'1 7'1 8'1|9'1
         *     |               !                   |
         *  -'2|0'2 1'2 2'2 3'2!4'2 5'2 6'2 7'2 8'2|9'2
         *     |               !                   |
         *  -'3|0'3 1'3 2'3 3'3!4'3 5'3 6'3 7'3 8'3|9'3
         *     |               !                   |
         *  -'4|0'4 1'4 2'4 3'4!4'4 5'4 6'4 7'4 8'4|9'4
         *     |               !                   |
         *  -'5|0'5 1'5 2'5 3'5!4'5 5'5 6'5 7'5 8'5|9'5
         *     |               !                   |
         *  -'6|0'6 1'6 2'6 3'6!4'6 5'6 6'6 7'6 8'6|9'6
         *     |               !                   |
         *  -'7|0'7 1'7 2'7 3'7!4'7 5'7 6'7 7'7 8'7|9'7
         *     |               !                   |
         *  -'8|0'8 1'8 2'8 3'8!4'8 5'8 6'8 7'8 8'8|9'8
         *      -----------------------------------
         *
         *
         *
         */


    static constexpr uint32_t bitindex(Position p)
    {
        auto ret = uint32_t(BIT_OFFSET + BIT_PER_COLUMN * p.x + p.y);
        //        std::cerr << p << "->" << ret << std::endl;
        return ret;
    }

    static constexpr Position invindex(uint32_t i)
    {
        uint32_t j = i - BIT_OFFSET;
        return {int8_t(j / BIT_PER_COLUMN), int8_t(j % BIT_PER_COLUMN)};
    }
    bool constexpr operator[](Position p) const
    {
        uint32_t i = bitindex(p);
        return getBit(data[i / 64], (i % 64));
    }

    void constexpr set(Position p, bool val)
    {
        uint32_t i = bitindex(p);
        return setBit(data[i / 64], (i % 64), val);
    }

    BitBoard constexpr dshift(int n) const
    {
        return {data[0] << n, data[1] << n};
    }
    BitBoard constexpr ushift(int n) const
    {
        return {{data[0] >> n, data[1] >> n}};
    }

    BitBoard constexpr rshift(int n) const
    {
        int shift = (n * BIT_PER_COLUMN);
        return {{data[0] << shift, data[1] << shift | data[0] >> (64 - shift)}};
    }
    BitBoard constexpr lshift(int n) const
    {
        int shift = (n * BIT_PER_COLUMN);
        return {{data[0] >> shift | data[1] << (64 - shift), data[1] >> shift}};
    }

    BitBoard constexpr operator&(const BitBoard& bb) const
    {
        return {{data[0] & bb.data[0], data[1] & bb.data[1]}};
    }

    BitBoard constexpr operator|(const BitBoard& bb) const
    {
        return {{data[0] | bb.data[0], data[1] | bb.data[1]}};
    }

    BitBoard constexpr operator^(const BitBoard& bb) const
    {
        return {{data[0] ^ bb.data[0], data[1] ^ bb.data[1]}};
    }
    BitBoard constexpr operator~() const
    {
        return {{~data[0], ~data[1]}};
    }
    constexpr bool operator==(const BitBoard& p2) const
    {
        return data[0] == p2.data[0] && data[1] == p2.data[1];
    }
    constexpr bool operator!=(const BitBoard& p2) const
    {
        return !(*this == p2);
    }


    constexpr BitBoard paraAdd(const BitBoard& p2) const
    {
        return {data[0] + p2.data[0], data[1] + p2.data[1]};
    }

    constexpr BitBoard paraSub(const BitBoard& p2) const
    {
        return {data[0] - p2.data[0], data[1] - p2.data[1]};
    }
    constexpr bool empty() const
    {
        return *this == BitBoard{0ull, 0ull};
    }

    static constexpr BitBoard oneHot(Position p)
    {
        BitBoard ret = {{0ull, 0ull}};
        int i = bitindex(p);
        ret.data[i / 64] = 1ull << (i % 64);
        return ret;
    }
    template <class F>
    constexpr void forEachBit(const F& func)
    {
        for (int i = 0; i < 2; i++) {
            uint64_t rem = data[i];
            while (rem != 0) {
                uint32_t bit = __builtin_ctzll(rem);
                func(invindex(bit + i * 64));
                rem &= ~(1ull << bit);
            }
        }
    }
};


//@formatter:off
// clang-format off
constexpr BitBoard my_goal{
        /*9876543210 9876543210 9876543210 9876543210 9876543210 dcba9876543210*/
        0b0000000010'0000000010'0000000010'0000000010'0000000000'00000000000000ull,
        /*3210 9876543210 9876543210 9876543210 9876543210 9876543210 9876543210*/
        0b0000'0000000000'0000000010'0000000010'0000000010'0000000010'0000000010ull
        };
//                            0b00000000000000'0000000000'0100000000'0100000000'0100000000'0100000000ull,
//                            /*0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123*/
//                            0b0100000000'0100000000'0100000000'0100000000'0100000000'0000000000'0000ull


constexpr BitBoard opponent_goal{
        /*9876543210 9876543210 9876543210 9876543210 9876543210 dcba9876543210*/
        0b1000000000'1000000000'1000000000'1000000000'0000000000'00000000000000ull,
        /*3210 9876543210 9876543210 9876543210 9876543210 9876543210 9876543210*/
        0b0000'0000000000'1000000000'1000000000'1000000000'1000000000'1000000000ull
        };

//    /*0123456789abcd 0123456789 0123456789 0123456789 0123456789 0123456789*/
//                                 0b00000000000000'0000000000'0000000001'0000000001'0000000001'0000000001ull,
//                                 /*0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123*/
//                                 0b0000000001'0000000001'0000000001'0000000001'0000000001'0000000000'0000ull
//                                };

constexpr BitBoard possible_wall_place{
        /*9876543210 9876543210 9876543210 9876543210 9876543210 dcba9876543210*/
        0b0111111110'0111111110'0111111110'0111111110'0000000000'00000000000000ull,
        /*3210 9876543210 9876543210 9876543210 9876543210 9876543210 9876543210*/
        0b0000'0000000000'0111111110'0111111110'0111111110'0111111110'0111111110ull
        };


//    /*0123456789abcd 0123456789 0123456789 0123456789 0123456789 0123456789*/
//                                       0b00000000000000'0000000000'0111111110'0111111110'0111111110'0111111110ull,
//                                       /*0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123*/
//                                       0b0111111110'0111111110'0111111110'0111111110'0000000000'0000000000'0000ull
//                                      };


constexpr BitBoard initial_horizontal{
        /*9876543210 9876543210 9876543210 9876543210 9876543210 dcba9876543210*/
        0b1000000001'1000000001'1000000001'1000000001'0000000000'00000000000000ull,
        /*3210 9876543210 9876543210 9876543210 9876543210 9876543210 9876543210*/
        0b0000'0000000000'0000000000'1000000001'1000000001'1000000001'1000000001ull
        };

constexpr BitBoard initial_vertical{
        /*9876543210 9876543210 9876543210 9876543210 9876543210 dcba9876543210*/
        0b0000000000'0000000000'0000000000'0000000000'0111111110'00000000000000ull,
        /*3210 9876543210 9876543210 9876543210 9876543210 9876543210 9876543210*/
        0b0000'0000000000'0111111110'0000000000'0000000000'0000000000'0000000000ull
        };

// clang-format on
//@formatter:on

constexpr BitBoard flipWalls(const BitBoard& walls)
{
    //@formatter:off
    // clang-format off
    constexpr uint64_t m[][7] = {
             /*9876543210 9876543210 9876543210 9876543210 9876543210 dcba9876543210*/
            {0b0000011111'0000011111'0000011111'0000011111'0000011111'00000000000000ull,
             0b1111100000'1111100000'1111100000'1111100000'1111100000'00000000000000ull,
             0b0001100011'0001100011'0001100011'0001100011'0001100011'00000000000000ull,
             0b1100011000'1100011000'1100011000'1100011000'1100011000'00000000000000ull,
             0b0100101001'0100101001'0100101001'0100101001'0100101001'00000000000000ull,
             0b1001010010'1001010010'1001010010'1001010010'1001010010'00000000000000ull,
             0b0010000100'0010000100'0010000100'0010000100'0010000100'00000000000000ull},
             /*3210 9876543210 9876543210 9876543210 9876543210 9876543210 9876543210*/
            {0b0000'0000011111'0000011111'0000011111'0000011111'0000011111'0000011111ull,
             0b0000'1111100000'1111100000'1111100000'1111100000'1111100000'1111100000ull,
             0b0000'0001100011'0001100011'0001100011'0001100011'0001100011'0001100011ull,
             0b0000'1100011000'1100011000'1100011000'1100011000'1100011000'1100011000ull,
             0b0000'0100101001'0100101001'0100101001'0100101001'0100101001'0100101001ull,
             0b0000'1001010010'1001010010'1001010010'1001010010'1001010010'1001010010ull,
             0b0000'0010000100'0010000100'0010000100'0010000100'0010000100'0010000100ull}
    };
    // clang-format on
    //@formatter:on
    BitBoard ret{};
    for (int i = 0; i < 2; i++) {
        uint64_t x0 = walls.data[i];
        uint64_t x1 = (x0 & m[i][0]) << 5 | (x0 & m[i][1]) >> 5;
        uint64_t x2 = (x1 & m[i][2]) << 3 | (x1 & m[i][3]) >> 3;
        uint64_t x3 = (x2 & m[i][4]) << 1 | (x2 & m[i][5]) >> 1 | (x1 & m[i][6]);
        ret.data[i] = x3;
    }
    return ret;
}

constexpr int potentialSearch(const BitBoard& hfullwall, const BitBoard& vfullwall, const BitBoard& start, const BitBoard& goal)
{
    int cost = 0;

    auto pullDown = [&](const auto& p1) {
        return ((~(~hfullwall & p1).paraAdd(~hfullwall)) ^ hfullwall) | p1;  // 下に引っ張る(コスト0)
    };
    auto addCost = [&](const auto& p1, const auto& p2) {
        return p1 | (p1.lshift(1) & ~vfullwall) | (p1.rshift(1) & ~(vfullwall.rshift(1))) | (p2.ushift(1) & ~hfullwall);
    };
    auto check = [&](const auto& s) {
        return !(s & goal).empty();
    };

    BitBoard p0 = pullDown(start);
    BitBoard p1{};
    BitBoard p2{};


    auto oneRound = [&](auto& p0, auto& p1, auto& out) {
        if (check(p0))
            return true;
        auto temp = addCost(p0, p1);
        out = pullDown(temp);
        if (out == p1) {
            cost = -1;
            return true;
        }
        cost++;
        return false;
    };
    while (true) {
        if (oneRound(p0, p1, p2))
            return cost;
        if (oneRound(p2, p0, p1))
            return cost;
        if (oneRound(p1, p2, p0))
            return cost;
    }
}


struct State {


    //! この2つの型別名定義がnegascoutに必要
    using NextMove = std::variant<Advance, Put>;  //!< 次の手の型
    using NextState = State;                      //!< 次の状態の型。


    Position my_pos;        // 16bit
    Position opponent_pos;  // 16bit

    int16_t my_rem_walls;
    int16_t opponent_rem_walls;

    BitBoard wall_h;  // 端には囲うように壁を配置する。
    BitBoard wall_v;


    constexpr bool operator==(const State& s2) const
    {
        return 0 == std::memcmp(this, &s2, sizeof(State));
    }
    constexpr bool operator!=(const State& s2) const
    {
        return !(*this == s2);
    }

    constexpr bool hWall(Position p) const
    {
        return wall_h[p];  // 端には囲うように壁を配置する。境界チェックしていないことに注意。
    }
    constexpr bool vWall(Position p) const
    {
        return wall_v[p];  // 端には囲うように壁を配置する。境界チェックしていないことに注意。
    }
    constexpr State flipPlayers() const
    {
        return State{
            .my_pos = Position{opponent_pos.x, int8_t(N - 1 - opponent_pos.y)},
            .opponent_pos = Position{my_pos.x, int8_t(N - 1 - my_pos.y)},
            .my_rem_walls = opponent_rem_walls,
            .opponent_rem_walls = my_rem_walls,
            .wall_h = flipWalls(wall_h),
            .wall_v = flipWalls(wall_v)};
    }

    constexpr BitBoard hFullWall() const
    {
        return wall_h | wall_h.rshift(1);
    }
    constexpr BitBoard vFullWall() const
    {
        return wall_v | wall_v.dshift(1);
    }

    constexpr static State initialState()
    {

        /*
         *  14  24  34  44  54 !64  74  84  94  104 114
         *                     !
         *                     !
         *  -'- 0'- 1'- 2'- 3'-!4'- 5'- 6'- 7'- 8'- 9'-
         *      -----------------------------------
         *  -'0|0'0 1'0 2'0 3'0!4'0 5'0 6'0 7'0 8'0|9'0
         *     |               !                   |
         *  -'1|0'1 1'1 2'1 3'1!4'1 5'1 6'1 7'1 8'1|9'1
         *     |               !                   |
         *  -'2|0'2 1'2 2'2 3'2!4'2 5'2 6'2 7'2 8'2|9'2
         *     |               !                   |
         *  -'3|0'3 1'3 2'3 3'3!4'3 5'3 6'3 7'3 8'3|9'3
         *     |               !                   |
         *  -'4|0'4 1'4 2'4 3'4!4'4 5'4 6'4 7'4 8'4|9'4
         *     |               !                   |
         *  -'5|0'5 1'5 2'5 3'5!4'5 5'5 6'5 7'5 8'5|9'5
         *     |               !                   |
         *  -'6|0'6 1'6 2'6 3'6!4'6 5'6 6'6 7'6 8'6|9'6
         *     |               !                   |
         *  -'7|0'7 1'7 2'7 3'7!4'7 5'7 6'7 7'7 8'7|9'7
         *     |               !                   |
         *  -'8|0'8 1'8 2'8 3'8!4'8 5'8 6'8 7'8 8'8|9'8
         *      -----------------------------------
         *
         *
         *
         */

        return State{{4, 8}, {4, 0}, 10, 10, initial_horizontal, initial_vertical};
        //            //@formatter:off
        //            // clang-format off
        //                {/*0123456789abcd 0123456789 0123456789 0123456789 0123456789 0123456789*/
        //                 0b00000000000000'0000000000'1000000001'1000000001'1000000001'1000000001ull,
        //                 /*0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123*/
        //                 0b1000000001'1000000001'1000000001'1000000001'0000000000'0000000000'0000ull
        //                },
        //                {/*0123456789abcd 0123456789 0123456789 0123456789 0123456789 0123456789*/
        //                 0b00000000000000'0111111110'0000000000'0000000000'0000000000'0000000000ull,
        //                 /*0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123*/
        //                 0b0000000000'0000000000'0000000000'0000000000'0111111110'0000000000'0000ull
        //                }};
        //        // clang-format on
        //        //@formatter:on
    }

    constexpr bool hasWall(Position p, MoveDirection dir) const
    {
        switch (dir) {
        case MoveDirection::Right:
            return vFullWall()[p];
        case MoveDirection ::Up:
            return hFullWall()[p - Position{0, 1}];
        case MoveDirection ::Left:
            return vFullWall()[p - Position{1, 0}];
        case MoveDirection ::Down:
            return hFullWall()[p];
        default:
            return false;
        }
    }

    constexpr bool lose() const
    {
        return opponent_pos.y == N - 1;
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
                                          flipped.opponent_pos = Position{m.new_position.x, int8_t(N - 1 - m.new_position.y)};
                                          return flipped;
                                      }
                                  }
                                  return std::nullopt;
                              },
                              [&](const Put& m) -> std::optional<NextState> {
                                  if (my_rem_walls == 0          // 残り壁枚数チェック
                                      || !inRange(m.pos, N - 1)  // 範囲チェック
                                      || hWall(m.pos)
                                      || vWall(m.pos)  // 重複チェック
                                      || (m.direction == WallDirection::Horizontal ? hWall(m.pos - Position{1, 0}) : vWall(m.pos - Position{0, 1}))
                                      || (m.direction == WallDirection::Horizontal ? hWall(m.pos + Position{1, 0}) : vWall(m.pos + Position{0, 1}))) {  // 平行干渉チェック
                                      return std::nullopt;
                                  }

                                  State copy = *this;
                                  if (m.direction == WallDirection::Horizontal) {
                                      copy.wall_h.set(m.pos, true);
                                  } else {
                                      copy.wall_v.set(m.pos, true);
                                  }
                                  copy.my_rem_walls--;
                                  // 自分や相手が動けなくなるような置き方をしてはいけない
                                  if (potentialSearch(copy.hFullWall(), copy.vFullWall(), my_goal, BitBoard::oneHot(copy.my_pos)) == -1) {
                                      return std::nullopt;
                                  }
                                  if (potentialSearch(copy.hFullWall(), copy.vFullWall(), BitBoard::oneHot(copy.opponent_pos), opponent_goal) == -1) {
                                      return std::nullopt;
                                  }
                                  return State{copy.flipPlayers()};
                              }},
            m);
    }


    struct PrettyFormat;
    PrettyFormat pretty(bool flip = false) const;


    /*!
     * 動かせる全ての手を列挙し、関数オブジェクトfuncを呼ぶ。
     * @param state 現在の状態
     * @param func 呼ぶ関数オブジェクト
     */
    template <class F>
    void forEachMove(const F& func) const
    {
        BitBoard hfullwall = hFullWall();
        BitBoard vfullwall = vFullWall();

        State flipped = flipPlayers();
        {

            for (int i = 0; i < 4; i++) {
                auto call = [&](Position diff) {
                    Position newp = my_pos + rotate(diff, i);
                    NextMove m = Advance{newp};
                    flipped.opponent_pos = Position{newp.x, int8_t(N - 1 - newp.y)};
                    func(m, flipped);
                };

                auto has_wall = [&](Position p, int dn90 = 0) {
                    dn90 += i;
                    dn90 &= 3;
                    BitBoard& wall = ((dn90 & 1) == 0 ? vfullwall : hfullwall);
                    static constexpr Position displacement[] = {{0, 0}, {0, -1}, {-1, 0}, {0, 0}};
                    return wall[p + displacement[dn90]];
                };
                if (has_wall(my_pos))
                    continue;
                if (opponent_pos != my_pos + rotate({1, 0}, i)) {
                    call({1, 0});
                } else if (!has_wall(opponent_pos)) {
                    call({2, 0});
                } else {
                    if (!has_wall(opponent_pos, -1)) {
                        call({1, 1});
                    }
                    if (!has_wall(opponent_pos, 1)) {
                        call({1, -1});
                    }
                }
            }
        }

        flipped.opponent_pos = Position{my_pos.x, int8_t(N - 1 - my_pos.y)};
        if (my_rem_walls > 0) {
            auto call = [&func](WallDirection dir, Position p, const State& bb) {
                NextMove m = Put{dir, p};
                func(m, bb);
            };


            auto for_each_bit = [](const BitBoard& bb, const auto& func) {
                for (int i = 0; i < 2; i++) {
                    uint64_t rem = bb.data[i];
                    while (rem != 0) {
                        uint32_t bit = __builtin_ctzll(rem);
                        func(BitBoard::invindex(bit + i * 64), i, bit);
                        rem &= ~(1ull << bit);
                    }
                }
            };
            BitBoard h_possible = possible_wall_place & ~flipped.wall_h & ~flipped.wall_h.rshift(1) & ~flipped.wall_h.lshift(1) & ~flipped.wall_v;

            for_each_bit(h_possible, [&](Position p, int i, uint32_t bit) {
                flipped.wall_h.data[i] |= (1ull << bit);
                call(WallDirection::Horizontal, {p.x, int8_t(N - 2 - p.y)}, flipped);
                flipped.wall_h.data[i] &= ~(1ull << bit);
            });

            BitBoard v_possible = possible_wall_place & ~flipped.wall_v & ~flipped.wall_v.dshift(1) & ~flipped.wall_v.ushift(1) & ~flipped.wall_h;

            for_each_bit(v_possible, [&](Position p, int i, uint32_t bit) {
                flipped.wall_v.data[i] |= (1ull << bit);
                call(WallDirection::Vertical, {p.x, int8_t(N - 2 - p.y)}, flipped);
                flipped.wall_v.data[i] &= ~(1ull << bit);
            });
        }
    }

    /*!
 * わかりやすい形で出力する。
 */
    std::ostream& printWith(std::ostream& os, const BitBoard& bb)
    {
        os << '|' << std::string(2 * N - 1, '-') << "|\n";
        for (int y = 0; y < N; y++) {

            os << '|';
            for (int x = 0; x < N; x++) {
                Position pos{int8_t(x), int8_t(y)};
                os << (bb[pos] ? '1' : '0')
                   << (hasWall(pos, MoveDirection::Right) ? '|' : ' ');
            }
            os << '\n';
            os << '|';
            for (int x = 0; x < N; x++) {
                Position pos{int8_t(x), int8_t(y)};
                os << (hasWall(pos, MoveDirection::Down) ? '-' : ' ')
                   << (hWall(pos) ? '-' : vWall(pos) ? '|' : ' ');
            }
            os << '\n';
        }
        return os;
    }
};

struct State::PrettyFormat : State {
    bool flip;
};

inline State::PrettyFormat State::pretty(bool flip) const
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
            Position pos{int8_t(x), int8_t(y)};
            bool movable = s.move(Advance{s.flip ? Position{pos.x, int8_t(N - 1 - pos.y)} : pos}).has_value();
            os << (s2.my_pos == pos ? '^' : s2.opponent_pos == pos ? 'v' : movable ? 'o' : '.')
               << (s2.hasWall(pos, MoveDirection::Right) ? '|' : ' ');
        }
        os << '\n';
        os << '|';
        for (int x = 0; x < N; x++) {
            Position pos{int8_t(x), int8_t(y)};
            os << (s2.hasWall(pos, MoveDirection::Down) ? '-' : ' ')
               << (s2.hWall(pos) ? '-' : s2.vWall(pos) ? '|' : ' ');
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
            Position pos{int8_t(x), int8_t(y)};
            os << (s.hWall(pos) ? 1 : s.vWall(pos) ? 2 : 0) << (x == N - 2 ? '\n' : ' ');
        }
    }
    return os;
}

std::istream& operator>>(std::istream& is, State& s)
{
    s = State::initialState();
    is >> s.my_pos >> s.opponent_pos >> s.my_rem_walls >> s.opponent_rem_walls;
    for (int y = 0; y < N - 1; y++) {
        for (int x = 0; x < N - 1; x++) {
            Position p = {int8_t(x), int8_t(y)};
            int w;
            is >> w;

            if (w == 1) {
                s.wall_h.set(p, true);
            } else if (w == 2) {
                s.wall_v.set(p, true);
            }
        }
    }
    return is;
}
template <class T, class SFINAE = std::enable_if_t<sizeof(State) % sizeof(uint64_t) == 0>>
static constexpr uint32_t hash(const T& s)
{
    const uint64_t* ptr = reinterpret_cast<const uint64_t*>(&s);
    uint64_t temp = 0;
    for (int i = 0; i < int(sizeof(State) / sizeof(uint64_t)); i++) {
        temp ^= ptr[i];
    }
    return uint32_t(temp) ^ uint32_t(temp >> 32);
}

}  // namespace QuoridorFast
