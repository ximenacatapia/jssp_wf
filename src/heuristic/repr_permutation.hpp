#ifndef JSSP_REPR_PERMUTATION_HPP
#define JSSP_REPR_PERMUTATION_HPP

#include <random>

#include "../eval/cost.hpp"
#include "../eval/schedule.hpp"
#include "../model/instance.hpp"
#include "../model/solution.hpp"
#include "neighborhood.hpp"
#include "representation.hpp"

/* Representation adapter: the operation-based permutation (Solution).
    This representation is ALWAYS feasible, so the start mode is ignored.
*/

namespace jssp
{
    // Create a random permutation. Always feasible, so mode is ignored.
    template <>
    inline Solution repr_random<Solution>(const Instance &inst,
                                          std::mt19937 &rng,
                                          StartMode /*mode*/)
    {
        return Solution::random(inst, rng);
    }

    // apply one random swap move because of neighbor
    inline Solution repr_neighbor(const Instance &inst, const Solution &sol,
                                  std::mt19937 &rng)
    {
        (void)inst;
        SwapMove mv = random_move(sol, rng);
        return apply_move(sol, mv);
    }

    inline double repr_cost(const Instance &inst, const Solution &sol)
    {
        return cost(inst, sol);
    }

    inline int repr_makespan(const Instance &inst, const Solution &sol)
    {
        return makespan_of(inst, sol);
    }

    // Always feasible, no operation is ever in a cycle.
    inline int repr_nodes_in_cycle(const Instance & /*inst*/,
                                   const Solution & /*sol*/)
    {
        return 0;
    }
} // namespace jssp

#endif // JSSP_REPR_PERMUTATION_HPP
