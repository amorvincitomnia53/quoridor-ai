#pragma once
#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include <random>
#include <vector>


using TScore = int;
constexpr TScore INF = 2000000000;

template <class TState>
using Move = typename TState::NextMove;

template <class TState>
using NextState = typename TState::NextState;

template <class TState>
struct Entry {
    TScore score;
    Move<TState> move;
    NextState<TState> state;
};
struct Abort {
};

template <class TState, class FEval, class F>
void forEachSortedNextMoves(const TState& state, const FEval& eval, const F& func, int best_path_id)
{

    std::vector<Entry<TState>> next_entries;
    next_entries.reserve(1024);
    //    std::cerr<<"\n";
    forEachMove(state, [&](const Move<TState>& move, const NextState<TState>& next_state) {
        //        std::cerr << eval(next_state)<<" "<<move << "\n" << next_state<< std::endl;
        next_entries.push_back(Entry<TState>{eval(next_state), move, next_state});
    });
    std::vector<Entry<TState>*> next_entry_ptrs(next_entries.size());
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
    for (auto* ptr : next_entry_ptrs) {
        auto& [score, next_move, next_state] = *ptr;
        bool cont = func(score, next_move, next_state, ptr - next_entries.data(), false);
        if (!cont)
            break;
    }
}

int best_path_table[20][20];
template <class TState, bool first = true>
struct Result {
    void setDepth(int depth)
    {
        this->depth = depth;
    }
    int depth;
    TScore score;
    /*std::vector<*/ Move<TState> /*>*/ move;

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


#ifdef NOVERBOSE
constexpr int verbose_depth = -1;
#else
constexpr int verbose_depth = 5;
#endif
constexpr int verbose_search_depth = 7;

/*
template <bool depth0, class TState, class FEval, class FStop>
std::conditional_t<depth0, Result<TState>, TScore> negaalpha(const TState& state, const FEval& eval, TScore score0, int depth, int search_depth, TScore alpha, TScore beta, bool is_best_path, int table_depth, const FStop& stop)
{

    if (stop()) {
        throw Abort{};
    }
    if (std::abs(score0) == INF || depth == search_depth) {
        //        std::cerr << depth << " " << score0 << "\n"
        //                  << state << "\n";
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
        return {score0};
#pragma GCC diagnostic pop
    }

    if constexpr (depth0) {

        if (depth <= verbose_depth && search_depth <= verbose_search_depth) {
            std::cerr << std::string(depth, '\t') << "[" << std::endl;
        }
        Result<TState> ret = {};
        ret.depth = search_depth;
        bool first = true;
        forEachSortedNextMoves(state, eval,
            [&](TScore score, const Move<TState>& move, const NextState<TState>& next_state, int index, bool is_best) {
                auto neg_score = negaalpha<false>(next_state, eval, score, depth + 1, search_depth, -beta, -alpha + 1, is_best, table_depth, stop);

                if (depth <= verbose_depth && search_depth <= verbose_search_depth) {
                    std::cerr << std::string(depth + 1, '\t') << (is_best ? "*" : "") << index << "(" << move << "): " << -neg_score;
                    ;
                }

                if (alpha < -neg_score) {
                    if (depth <= verbose_depth && search_depth <= verbose_search_depth) {
                        std::cerr << "!";
                    }
                    alpha = -neg_score;
                    ret.moves.clear();
                    ret.moves.push_back(move);

                    best_path_table[depth][depth] = index;
                    std::memcpy(&best_path_table[depth][depth + 1], &best_path_table[depth + 1][depth + 1], sizeof(int) * (search_depth - depth - 1));

                } else if (first) {
                    if (depth <= verbose_depth && search_depth <= verbose_search_depth) {
                        std::cerr << "%";
                    }
                    //                    ret.moves.push_back(move);
                    best_path_table[depth][depth] = index;
                    std::memcpy(&best_path_table[depth][depth + 1], &best_path_table[depth + 1][depth + 1], sizeof(int) * (search_depth - depth - 1));
                } else if (alpha == -neg_score) {
                    ret.moves.push_back(move);
                    if (depth <= verbose_depth && search_depth <= verbose_search_depth) {
                        std::cerr << "$";
                        std::cerr << "!";
                        //                        std::cerr << depth << "/" << search_depth << " " << move << " " << -neg_score << std::endl;
                    }
                }
                if (depth <= verbose_depth && search_depth <= verbose_search_depth) {
                    std::cerr << std::endl;
                }
                first = false;
                return alpha < beta;
            },
            (is_best_path && depth < table_depth) ? best_path_table[0][depth] : -1);


        if (depth <= verbose_depth && search_depth <= verbose_search_depth) {
            std::cerr << std::string(depth, '\t') << "]" << std::endl;
        }
        ret.score = alpha;

        return ret;
    } else {

        if (depth <= verbose_depth && search_depth <= verbose_search_depth) {
            std::cerr << std::string(depth, '\t') << "[" << std::endl;
        }
        forEachSortedNextMoves(state, eval,
            [&](TScore score, const Move<TState>& move, const NextState<TState>& next_state, int index, bool is_best) {
                auto neg_score = negaalpha<false>(next_state, eval, score, depth + 1, search_depth, -beta, -alpha, is_best, table_depth, stop);

                if (depth <= verbose_depth && search_depth <= verbose_search_depth) {
                    std::cerr << std::string(depth + 1, '\t') << (is_best ? "*" : "") << index << "(" << move << "): " << -neg_score;
                }
                if (alpha < -neg_score) {
                    alpha = -neg_score;
                    if (depth <= verbose_depth && search_depth <= verbose_search_depth) {
                        std::cerr << "!" << std::endl;
                    }
                    best_path_table[depth][depth] = index;

                    std::memcpy(&best_path_table[depth][depth + 1], &best_path_table[depth + 1][depth + 1], sizeof(int) * (search_depth - depth - 1));
                } else {

                    if (depth <= verbose_depth && search_depth <= verbose_search_depth) {
                        std::cerr << std::endl;
                    }
                }
                return alpha < beta;
            },
            (is_best_path && depth < table_depth) ? best_path_table[0][depth] : -1);

        if (depth <= verbose_depth && search_depth <= verbose_search_depth) {
            std::cerr << std::string(depth, '\t') << "]" << std::endl;
        }
        return alpha;
    }
}
*/


template <bool first_, class TState, class FEval, class FStop>
Result<TState, first_> negascout(const TState& state, const FEval& eval, TScore score0, int depth, int search_depth, TScore alpha, TScore beta, bool is_best_path, int table_depth, const FStop& stop)
{
#define DEBUG(...)                                                        \
    if (depth <= verbose_depth && search_depth <= verbose_search_depth) { \
        std::cerr << __VA_ARGS__;                                         \
    }

    if (stop()) {
        throw Abort{};
    }

    Result<TState, first_> ret = {};
    ret.setDepth(search_depth);
    if (std::abs(score0) == INF || depth == search_depth) {

        //        if (eval(state) == -INF) {
        //            std::cerr << state << std::endl;
        //        std::cerr << eval(state)<< std::endl;
        //        }
        //        DEBUG(std::string(depth, '\t') << "[ <fin>" << score0 << " ]\n");

        ret.score = score0;
        return ret;
    }

    DEBUG(std::string(depth, '\t') << "[" << std::endl)

    ret.score = -INF - 1;
    bool first = true;
    forEachSortedNextMoves(state, eval,
        [&](TScore eval_score, const Move<TState>& m, const NextState<TState>& next_state, int index, bool is_best) {
            auto registerBestMove = [&](const auto& m, TScore score) {
                if (ret.score < score) {
                    ret.setMove(m);
                    ret.score = score;
                    best_path_table[depth][depth] = index;
                    std::memcpy(&best_path_table[depth][depth + 1], &best_path_table[depth + 1][depth + 1], sizeof(int) * (search_depth - depth - 1));
                }
            };

            TScore score = first ? alpha : -negascout<false>(next_state, eval, eval_score, depth + 1, search_depth, -alpha - 1, -alpha, is_best, table_depth, stop).score;

            if (beta <= score) {
                registerBestMove(m, score);
                DEBUG(std::string(depth + 1, '\t') << "!!" << m << ": " << score << "+" << std::endl);
                return false;
            }
            if (alpha < score || first) {
                alpha = score;
                score = -negascout<false>(next_state, eval, eval_score, depth + 1, search_depth, -beta, -alpha, is_best, table_depth, stop).score;
                if (beta <= score) {
                    registerBestMove(m, score);
                    DEBUG(std::string(depth + 1, '\t') << "!!!" << m << ": " << score << "+" << std::endl);
                    return false;
                }
                if (alpha < score)
                    alpha = score;
            }
            registerBestMove(m, score);
            DEBUG(std::string(depth + 1, '\t') << m << ": " << score << std::endl);
            first = false;
            return true;
        },
        (is_best_path && depth < table_depth) ? best_path_table[0][depth] : -1);


    DEBUG(std::string(depth, '\t') << "]" << std::endl)

    if (ret.score == -INF - 1)
        ret.score = -INF;
    return ret;
}

template <class TState, class FEval>
auto negascout(const TState& state, const FEval& eval, int depth)
{
    return negascout<true>(state, eval, eval(state), 0, depth, -INF, INF, false, 0, [] { return false; });
}


template <class TState, class FEval, class FStop>
auto iterativeDeepeningNegaalpha(const TState& state, const FEval& eval, int depth, int last_depth, const FStop& stop)
{
    return negascout<true>(state, eval, eval(state), 0, depth, -INF, INF, true, last_depth, stop);
}