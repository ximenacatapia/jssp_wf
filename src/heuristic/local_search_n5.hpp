#ifndef JSSP_LOCAL_SEARCH_N5_HPP
#define JSSP_LOCAL_SEARCH_N5_HPP

#include <random>

#include "../eval/cost.hpp"
#include "../eval/critical.hpp"
#include "../eval/eval_counter.hpp"
#include "../model/instance.hpp"
#include "../model/solution.hpp"
#include "local_search.hpp"

/*
N5 local search for the permutation representation.
*/

namespace jssp
{

    // Counted cost for the N5 search
    inline double n5_cost(const Instance &inst, const Solution &sol)
    {
        bump_eval_count();
        return cost(inst, sol);
    }

    // Pure N5 steepest descent
    inline Solution n5_descent(const Instance &inst, const Solution &start,
                               int max_steps)
    {
        Solution current = start;
        double current_cost = n5_cost(inst, current);

        for (int step = 0; step < max_steps; ++step)
        {
            std::vector<N5Move> moves = n5_moves(inst, current);
            if (moves.empty())
                break;

            double best_cost = current_cost;
            int best_i = -1;
            for (int i = 0; i < static_cast<int>(moves.size()); ++i)
            {
                Solution cand = current;
                cand.swap_positions(moves[i].pos_a, moves[i].pos_b);
                double c = n5_cost(inst, cand);
                if (c < best_cost)
                {
                    best_cost = c;
                    best_i = i;
                }
            }

            if (best_i < 0)
                break; // no N5 move improves

            current.swap_positions(moves[best_i].pos_a, moves[best_i].pos_b);
            current_cost = best_cost;
        }
        return current;
    }

#ifndef JSSP_DISABLE_N5_OVERLOAD
    // N5 TABU SEARCH (Nowicki-Smutnicki style). N5 as the main engine.
    inline Solution n5_tabu_search(const Instance &inst,
                                   const Solution &start,
                                   const LocalSearchParams &params,
                                   std::mt19937 &rng)
    {
        (void)rng;

        Solution current = start;
        double current_cost = n5_cost(inst, current);

        Solution best = current; // best seen so far
        double best_cost = current_cost;

        const int tenure = 10;
        std::vector<std::pair<int, int>> tabu;
        auto is_tabu = [&](int a, int b)
        {
            for (const auto &t : tabu)
                if ((t.first == a && t.second == b) ||
                    (t.first == b && t.second == a))
                    return true;
            return false;
        };

        const int iters = params.max_steps;
        int no_improve = 0;

        for (int it = 0; it < iters; ++it)
        {
            std::vector<N5Move> moves = n5_moves(inst, current);
            if (moves.empty())
                break;

            int best_i = -1;
            double best_move_cost = 1e300;
            for (int i = 0; i < static_cast<int>(moves.size()); ++i)
            {
                Solution cand = current;
                cand.swap_positions(moves[i].pos_a, moves[i].pos_b);
                double c = n5_cost(inst, cand);

                bool tabu_move = is_tabu(moves[i].pos_a, moves[i].pos_b);
                bool aspires = c < best_cost; // beats global best
                if (tabu_move && !aspires)
                    continue;

                if (c < best_move_cost)
                {
                    best_move_cost = c;
                    best_i = i;
                }
            }

            if (best_i < 0)
                break;

            current.swap_positions(moves[best_i].pos_a, moves[best_i].pos_b);
            current_cost = best_move_cost;

            tabu.push_back({moves[best_i].pos_a, moves[best_i].pos_b});
            if (static_cast<int>(tabu.size()) > tenure)
                tabu.erase(tabu.begin());

            if (current_cost < best_cost)
            {
                best = current;
                best_cost = current_cost;
                no_improve = 0;
            }
            else if (++no_improve > iters / 2)
            {
                break;
            }
        }

        return best;
    }

    // make the generic local_search<Solution> run the
    // N5 tabu search
    template <>
    inline Solution local_search<Solution>(const Instance &inst,
                                           const Solution &start,
                                           const LocalSearchParams &params,
                                           std::mt19937 &rng)
    {
        return n5_tabu_search(inst, start, params, rng);
    }
#endif // JSSP_DISABLE_N5_OVERLOAD

} // namespace jssp

#endif
