#ifndef JSSP_CRITICAL_HPP
#define JSSP_CRITICAL_HPP

#include <vector>

#include "../model/instance.hpp"
#include "../model/solution.hpp"
#include "schedule.hpp"

/*
Critical blocks and the N5 (Nowicki-Smutnicki) neighborhood.
*/

namespace jssp
{

    struct N5Move
    {
        int pos_a; // positions in the permutation to swap
        int pos_b;
    };

    // Generates the N5 moves for 'sol'. Decodes the schedule, extracts the
    // critical path, splits it into critical blocks, and emits the end swaps.
    // Returns an empty list when no improving critical move exists.
    std::vector<N5Move> n5_moves(const Instance &inst, const Solution &sol);

} // namespace jssp

#endif // JSSP_CRITICAL_HPP