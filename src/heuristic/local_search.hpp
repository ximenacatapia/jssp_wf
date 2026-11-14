#ifndef JSSP_LOCAL_SEARCH_HPP
#define JSSP_LOCAL_SEARCH_HPP

#include <random>

#include "../model/instance.hpp"
#include "../model/solution.hpp"

/*
Local search: the "gravity" / hill-sliding phase of the Water-Flow.

Given a drop (solution), slide it downhill to a local optimum: repeatedly
move to a better neighbor until no sampled neighbor improves.

Two strategies to choose from:
    - FIRST_IMPROVEMENT : move to the first sampled neighbor that improves.
    - STEEPEST          : scan all K sampled neighbors, move to the best.
*/

namespace jssp
{

    enum class LocalSearchStrategy
    {
        FIRST_IMPROVEMENT,
        STEEPEST,
    };

    struct LocalSearchParams
    {
        LocalSearchStrategy strategy = LocalSearchStrategy::FIRST_IMPROVEMENT;
        int sample_size = 200;
        int max_steps = 1000;
    };

    // Sampled-neighborhood local search
    template <class Repr>
    Repr sampled_local_search(const Instance &inst,
                              const Repr &start,
                              const LocalSearchParams &params,
                              std::mt19937 &rng)
    {
        Repr current = start;
        double current_cost = repr_cost(inst, current);

        for (int step = 0; step < params.max_steps; ++step)
        {
            double best_cost = current_cost;
            Repr best = current;
            bool found = false;

            // Sample K neighbors.
            for (int k = 0; k < params.sample_size; ++k)
            {
                Repr candidate = repr_neighbor(inst, current, rng);
                double c = repr_cost(inst, candidate);

                if (c < best_cost)
                {
                    best_cost = c;
                    best = candidate;
                    found = true;

                    // First-improvement
                    if (params.strategy == LocalSearchStrategy::FIRST_IMPROVEMENT)
                        break;
                    // Steepest
                }
            }

            if (!found)
                break; // local optimum

            current = best;
            current_cost = best_cost;
        }

        return current;
    }

    // Generic local search
    template <class Repr>
    Repr local_search(const Instance &inst,
                      const Repr &start,
                      const LocalSearchParams &params,
                      std::mt19937 &rng)
    {
        return sampled_local_search(inst, start, params, rng);
    }

    class Solution;
    template <>
    Solution local_search<Solution>(const Instance &inst,
                                    const Solution &start,
                                    const LocalSearchParams &params,
                                    std::mt19937 &rng);

} // namespace jssp

#endif // JSSP_LOCAL_SEARCH_HPP
