#ifndef JSSP_WATER_FLOW_HPP
#define JSSP_WATER_FLOW_HPP

#include <random>

#include "../instance.hpp"
#include "../solution.hpp"
#include "local_search.hpp"

/*
Water-Flow Algorithm (WFA) for the JSSP, adapted:

The search is organized as on the paper:

    drop of water           -> a solution (permutation)
    rain / precipitation    -> a population of random solutions
    gravity / hill-sliding  -> local search sliding a drop to a local optimum
    erosion                 -> intensified local search on a promising valley
    cloud / iteration       -> one cycle of (rain -> erosion)

We keep three memory lists: best (P0), uneroded(UE) local optim that can still intensify
, eroded(E) local optim that's been exhausted
*/

namespace jssp
{

    struct WaterFlowParams
    {
        int max_clouds = 500;       // main iterations/clouds
        int rain_drops = 100;       // random solutions generated per exploration
        int erode_top = 70;         // how many best valleys to erode per cloud
        int erosion_attempts = 100; // intensification rounds before a valley is spent

        // Local search used for the gravity phase
        LocalSearchParams gravity;

        // Local search used for the erosion phase
        LocalSearchParams erosion;

        WaterFlowParams()
        {
            gravity.strategy = LocalSearchStrategy::FIRST_IMPROVEMENT;
            gravity.sample_size = 200;
            gravity.max_steps = 500;

            erosion.strategy = LocalSearchStrategy::STEEPEST;
            erosion.sample_size = 400;
            erosion.max_steps = 500;
        }
    };

    struct WaterFlowResult
    {
        Solution best;
        int makespan;
        double cost;
        int iterations;
        int nodes_in_cycle;
    };

    inline bool is_feasible(const WaterFlowResult &r)
    {
        return r.nodes_in_cycle == 0;
    }

    // Runs the Water-Flow algorithm and returns the best solution found.
    WaterFlowResult water_flow(const Instance &inst,
                               const WaterFlowParams &params,
                               std::mt19937 &rng);

} // namespace jssp

#endif // JSSP_WATER_FLOW_HPP
