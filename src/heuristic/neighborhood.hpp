#ifndef JSSP_NEIGHBORHOOD_HPP
#define JSSP_NEIGHBORHOOD_HPP

#include <random>
#include <utility>
#include <vector>

#include "../model/solution.hpp"

//  Neighborhood operator: swap of two positions (adapted 2-opt).
//  A neighbor of a solution is obtained by swapping the job ids at two
//  positions i and j of the operation-based permutation.
namespace jssp
{

    // A move is the pair of positions to swap.
    struct SwapMove
    {
        int i;
        int j;
    };

    SwapMove random_move(const Solution &sol, std::mt19937 &rng);

    Solution apply_move(const Solution &sol, SwapMove move);

    std::vector<SwapMove> all_moves(const Solution &sol);

} // namespace jssp

#endif // JSSP_NEIGHBORHOOD_HPP
