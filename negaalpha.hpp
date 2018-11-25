#pragma once
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <iostream>
#include <random>
#include <vector>

using TScore = int;                 //! 評価関数の値の型
constexpr TScore INF = 2000000000;  //! 評価関数で「勝利確定」「敗北確定」を表すのに使う

template <class TState>
using Move = typename TState::NextMove;

template <class TState>
using NextState = typename TState::NextState;

struct Abort {
};

int search_depth = -1;  //! 現在の探索深度
int table_depth = -1;   //! 最良手優先探索用のテーブルをどこまで見るか(反復深化を使う場合、前回の探索の深度を入れる。)

constexpr int MAX_DEPTH = 40;
int best_path_table[MAX_DEPTH][MAX_DEPTH];  //! 最良手優先探索用のテーブル。最終的にはbest_path_table[0][...]に最良手のインデックスが集まってくる。

template <class TState, class FEval, class F>
void forEachSortedNextMoves(const TState& state, const FEval& eval, const F& func, int best_path_id)
{

    struct Entry {
        TScore score;
        Move<TState> move;
        NextState<TState> state;
    };

    std::vector<Entry> next_entries;
    next_entries.reserve(1024);
    state.forEachMove([&](const Move<TState>& move, const NextState<TState>& next_state) {

#ifndef NDEBUG
        auto nsopt = state.move(move);
        if (!nsopt) {
            std::cerr << state.pretty() << '\n'
                      << move << std::endl;
            std::abort();
        } else if (*nsopt != next_state) {
            std::cerr << state.pretty() << '\n'
                      << move << '\n'
                      << nsopt->pretty() << '\n'
                      << next_state.pretty() << std::endl;
            std::abort();
        }
#endif
        next_entries.push_back(Entry{eval(next_state), move, next_state});
    });
    std::vector<Entry*> next_entry_ptrs(next_entries.size());
    for (int i = 0; i < int(next_entries.size()); i++) {
        next_entry_ptrs[i] = &next_entries[i];
    }
    if (best_path_id != -1 && next_entry_ptrs.size() > /*best_path_id*/ 0) {
        next_entry_ptrs.at(best_path_id) = next_entry_ptrs.back();
        next_entry_ptrs.resize(next_entry_ptrs.size() - 1);


        auto [score, next_move, next_state] = next_entries[best_path_id];
        bool cont = func(score, next_move, next_state, best_path_id, true);
        if (!cont)
            return;
    }


    std::sort(begin(next_entry_ptrs), end(next_entry_ptrs),
        [&](const auto* a, const auto* b) { return a->score < b->score; });

    for (int i = 0; i < int(next_entry_ptrs.size()); i++) {
        auto* ptr = next_entry_ptrs[i];
        auto& [score, next_move, next_state] = *ptr;
        bool cont = func(score, next_move, next_state, ptr - next_entries.data(), false);
        if (!cont)
            break;
    }
}


/*!
 * 最上階層以外は結果にはスコアだけしか必要ないので、Moveを保存しないことで最適化されることを期待する
 * @tparam first 最上階層かどうかを指定する
 */
template <class TState, bool first = true>
struct Result {
    void setDepth(int depth)
    {
        this->depth = depth;
    }
    int depth;
    TScore score;
    Move<TState> move;
    void setMove(const Move<TState>& move)
    {
        this->move = move;
    }
};

template <class TState>
struct Result<TState, false> {
    TScore score;
    void setDepth(int) {}
    void setMove(const Move<TState>&) {}
};


int verbose_depth = -1;
int verbose_search_depth = -1;

template <bool first_, class TState, class FEval, class FStop>
Result<TState, first_> negascout(const TState& state, const FEval& eval, TScore score0, int depth, TScore alpha, TScore beta, bool is_best_path, const FStop& stop)
{

#ifdef NOVERBOSE
#define DEBUG(...)
#else
#define DEBUG(...)                                                                     \
    if (depth <= verbose_depth && search_depth <= verbose_search_depth) {              \
        std::cout << depth << "/" << search_depth << ">" << __VA_ARGS__ << std::flush; \
    }
#endif
    Result<TState, first_> ret = {};
    ret.setDepth(search_depth);
    if (std::abs(score0) == INF || depth == search_depth) {
        ret.score = score0;
        return ret;
    }

    if (depth < search_depth - 1 && stop()) {
        throw Abort{};
    }

    DEBUG(std::string(depth, '\t') << "[\n")

    ret.score = -INF - 100;
    bool first = true;
    forEachSortedNextMoves(state, eval,
        [&](TScore eval_score, const Move<TState>& m, const NextState<TState>& next_state, int index, bool is_best) {
            auto registerBestMove = [&](const auto& m, TScore score) {
                if (ret.score < score) {
                    ret.setMove(m);
                    ret.score = score;
                    best_path_table[depth][depth] = index;
                    std::memcpy(&best_path_table[depth][depth + 1], &best_path_table[depth + 1][depth + 1], sizeof(int) * (search_depth - depth - 1));
                    DEBUG(std::string(depth + 1, '\t') << "REWRITE: ");
#ifndef NOVERBOSE
                    if (depth <= verbose_depth && search_depth <= verbose_search_depth) {
                        for (int i = 0; i < search_depth; i++) {
                            if (i < depth) {
                                std::cout << "* ";
                            } else {
                                std::cout << best_path_table[depth][i] << " ";
                            }
                        }
                        std::cout << '\n'
                                  << std::endl;
                    }
#endif
                }
            };

            TScore score = first ? alpha : -negascout<false>(next_state, eval, eval_score, depth + 1, -alpha - 1, -alpha, is_best, stop).score;

            if (beta <= score) {
                registerBestMove(m, score);
                DEBUG((is_best ? '*' : ' ') << std::string(depth + 1, '\t') << "[" << index << "]" << m << ": " << score << "+ scout beta cut\n\n");
                return false;
            }
            DEBUG((is_best ? '*' : ' ') << std::string(depth + 1, '\t') << "[" << index << "]"
                                        << "scout: " << m << ": " << score << '\n');
            if (alpha < score || first) {
                alpha = score;

                score = -negascout<false>(next_state, eval, eval_score, depth + 1, -beta, -alpha, is_best, stop).score;

                if (beta <= score) {
                    registerBestMove(m, score);
                    DEBUG((is_best ? '*' : ' ') << std::string(depth + 1, '\t') << "[" << index << "]" << m << ": " << score << "+ beta cut\n\n");
                    return false;
                }
                if (alpha < score)
                    alpha = score;
            }
            registerBestMove(m, score);
            DEBUG((is_best ? '*' : ' ') << std::string(depth + 1, '\t') << "[" << index << "]" << m << ": " << score << "\n\n");
            first = false;
            return true;
        },
        (is_best_path && depth < table_depth) ? best_path_table[0][depth] : -1);


    DEBUG(std::string(depth, '\t') << "]\n\n")

    //    if (ret.score == -INF - 100)
    //        ret.score = -INF;
    return ret;
}

template <class TState, class FEval>
auto negascout(const TState& state, const FEval& eval, int depth)
{
    search_depth = depth;
    table_depth = 0;
    return negascout<true>(state, eval, eval(state), 0, -INF, INF, false, [] { return false; });
}


template <class TState, class FEval, class FStop>
auto iterativeDeepeningNegascout(const TState& state, const FEval& eval, int depth, int last_depth, const FStop& stop)
{
    search_depth = depth;
    table_depth = last_depth;
    //    return negascout<true>(state, eval, eval(state), 0, -INF, INF, true, stop);
    auto ret = negascout<true>(state, eval, eval(state), 0, -INF, INF, true, stop);
    //    std::cout << std::endl;
    return ret;
}