#include "local_search.hpp"

#include "../cost.hpp"
#include "neighborhood.hpp"

namespace jssp
{

    namespace
    {

        // Tries to find a better neighbor of 'current' by sampling,  returns false if no sampled neighbor improves.
        bool find_improving_neighbor(const Instance &inst,
                                     const Solution &current,
                                     double current_cost,
                                     const LocalSearchParams &params,
                                     std::mt19937 &rng,
                                     Solution &next,
                                     double &next_cost)
        {
            bool found = false;
            double best_cost = current_cost;
            Solution best = current;

            for (int k = 0; k < params.sample_size; ++k)
            {
                SwapMove mv = random_move(current, rng);
                Solution candidate = apply_move(current, mv);
                double c = cost(inst, candidate);

                if (c < best_cost)
                {
                    best_cost = c;
                    best = candidate;
                    found = true;

                    // First-improvement we stop at the first better neighbor.
                    if (params.strategy == LocalSearchStrategy::FIRST_IMPROVEMENT)
                    {
                        break;
                    }
                    // keep scanning the sample to find the best one.
                }
            }

            if (found)
            {
                next = best;
                next_cost = best_cost;
            }
            return found;
        }

    } // namespace

    Solution local_search(const Instance &inst,
                          const Solution &start,
                          const LocalSearchParams &params,
                          std::mt19937 &rng)
    {
        Solution current = start;
        double current_cost = cost(inst, current);

        for (int step = 0; step < params.max_steps; ++step)
        {
            Solution next = current;
            double next_cost = current_cost;

            if (!find_improving_neighbor(inst, current, current_cost, params, rng,
                                         next, next_cost))
            {
                break; // local optimum
            }

            current = next;
            current_cost = next_cost;
        }

        return current;
    }

} // namespace jssp
