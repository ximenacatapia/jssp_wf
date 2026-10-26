#ifndef JSSP_NEIGHBORHOOD_HPP
#define JSSP_NEIGHBORHOOD_HPP

#include <random>
#include <utility>
#include <vector>

#include "../solution.hpp"

// ============================================================================
//  Neighborhood operator: swap of two positions (adapted 2-opt).
// ----------------------------------------------------------------------------
//  A neighbor of a solution is obtained by swapping the job ids at two
//  positions i and j of the operation-based permutation.
//
//  Key property: a swap only reorders job ids, so the multiset is unchanged
//  (each job still appears exactly m times). Therefore EVERY neighbor is valid
//  and decodes to a feasible schedule. No repair and no penalty are ever
//  needed. This is exactly why the operation-based representation was chosen.
//
//  A swap between two positions holding the SAME job id produces an identical
//  permutation (a "null move"). Those are skipped: the generators below only
//  ever return moves that change the sequence.
//
//  The Water-Flow algorithm uses this operator both for the hill-climbing
//  ("gravity") phase and for the erosion (intensification) phase.
// ============================================================================

namespace jssp
{

    // A move is the pair of positions to swap.
    struct SwapMove
    {
        int i;
        int j;
    };

    // Returns a random move that actually changes the sequence, i.e. the two
    // positions hold different job ids. Assumes the solution is not uniform (it
    // has at least two distinct ids, which always holds for n >= 2). The engine is
    // used to pick positions.
    SwapMove random_move(const Solution &sol, std::mt19937 &rng);

    // Applies a move to a copy of 'sol' and returns the resulting neighbor.
    // 'sol' is left unchanged.
    Solution apply_move(const Solution &sol, SwapMove move);

    // Enumerates every distinct-id swap (i < j) of the solution. The number of
    // moves is O((n*m)^2); use deliberately. Each entry is a move that changes the
    // sequence.
    std::vector<SwapMove> all_moves(const Solution &sol);

} // namespace jssp

#endif // JSSP_NEIGHBORHOOD_HPP
