#ifndef JSSP_LOCAL_SEARCH_HPP
#define JSSP_LOCAL_SEARCH_HPP

#include <random>

#include "../instance.hpp"
#include "../solution.hpp"

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

    Solution local_search(const Instance &inst,
                          const Solution &start,
                          const LocalSearchParams &params,
                          std::mt19937 &rng);

} // namespace jssp

#endif // JSSP_LOCAL_SEARCH_HPP
