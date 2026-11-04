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
        int sample_size = 200; // K: neighbors sampled per step
        int max_steps = 1000;
    };

    // Slides 'start' downhill to a local optimum and returns it. The returned
    // solution's cost is never worse than that of 'start'
    template <class Repr>
    Repr local_search(const Instance &inst,
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

                    // Foe the first-improvemen we stop at the first better neighbor
                    if (params.strategy == LocalSearchStrategy::FIRST_IMPROVEMENT)
                        break;
                    // keep scanning the sample for the best.
                }
            }

            if (!found)
                break; // local optimum

            current = best;
            current_cost = best_cost;
        }

        return current;
    }

} // namespace jssp

#endif // JSSP_LOCAL_SEARCH_HPP
