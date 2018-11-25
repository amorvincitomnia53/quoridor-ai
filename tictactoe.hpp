#pragma once
#include <optional>


/*!
 * 三目並べをするゲームクラス(negaalphaアルゴリズムのテスト用)
 */
namespace TicTacToe
{

/*!
 * 手
 * x, yで置く位置を指定する。turnは置くプレイヤーを指定する(1か-1)
 */
template <int turn>
struct Move {
    int x, y;


    /*!
     * 手をわかりやすい形で出力する。std::cout << move; みたいに呼べるようにする。
     */
    friend std::ostream& operator<<(std::ostream& os, const Move& m)
    {
        os << "<" << m.x << " " << m.y << ">";
        return os;
    }
};

/*!
 * 盤面の状態
 * boardが3x3の二次元配列になっている。それぞれの要素が-1, 0, 1のどれかで、0が置かれていない状態、-1と1は置いた人の番号である。
 * negascoutに必要な最低限に近い実装を示している。
 * @tparam turn_ 手番の人
 */
template <int turn_>
struct State {

    //! 評価関数から手番のプレイヤーを指定できるようにするための定数
    constexpr int turn = turn_;

    //! この2つの型別名定義がnegascoutに必要
    using NextMove = Move<turn>;     //!< 次の手の型
    using NextState = State<-turn>;  //!< 次の状態の型。手番が入れ替わっていることに注意

    //! 盤面。0(未着手)または-1 or 1(プレイヤー番号)
    std::array<std::array<int, 3>, 3> board = {};


    /*!
     * この状態から指定された手をうつ。その手が有効かどうかの判定も行う。
     * @param m 手
     * @return その手が有効であれば次の状態、無効であればstd::nulloptを返す。
     */
    std::optional<NextState> move(const NextMove& m)
    {
        if (s[m.y][m.x] != 0)
            return std::nullopt;
        NextState s{board};
        s[m.y][m.x] = turn;
        return s;
    }

    /*!
     * 動かせる全ての手を列挙し、関数オブジェクトfuncを呼ぶ。
     * @param state 現在の状態
     * @param func 呼ぶ関数オブジェクト
     */
    template <class F>
    void forEachMove(const F& func)
    {
        for (int y = 0; y <= 2; y++) {
            for (int x = 0; x <= 2; x++) {
                if (board[y][x] == 0) {
                    NextMove m{x, y};
                    func(m, *move(m));
                }
            }
        }
    }


    /*!
     * わかりやすい形で出力する。
     */
    friend std::ostream& operator<<(std::ostream& os, const State& s)
    {
        for (int y = 0; y <= 2; y++) {
            for (int x = 0; x <= 2; x++) {
                os << ("x.o"[1 + s.board[y][x]]);
            }
            os << '\n';
        }
        return os;
    }
};

}  // namespace TicTacToe