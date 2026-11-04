#ifndef JSSP_WATER_FLOW_HPP
#define JSSP_WATER_FLOW_HPP

#include <random>

#include "../model/instance.hpp"
#include "local_search.hpp"
#include "representation.hpp"

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
        int max_clouds = 1000;    // main iterations/clouds
        int rain_drops = 15;      // random solutions generated per exploration
        int erode_top = 5;        // how many best valleys to erode per cloud
        int erosion_attempts = 3; // intensification rounds before a valley is spent

        StartMode start = StartMode::NONFEASIBLE; // we start from non-feasible as defaulr

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

    // Result holding the best solution plus metrics.
    template <class Repr>
    struct WaterFlowResult
    {
        Repr best;
        int makespan;
        double cost;
        int iterations;
        int nodes_in_cycle; // 0 -> feasible
    };

    template <class Repr>
    bool is_feasible(const WaterFlowResult<Repr> &r)
    {
        return r.nodes_in_cycle == 0;
    }

    // Runs the Water-Flow algorithm on representation Repr and returns the best
    // solution found. Repr must provide the representation free functions.
    template <class Repr>
    WaterFlowResult<Repr> water_flow(const Instance &inst,
                                     const WaterFlowParams &params,
                                     std::mt19937 &rng)
    {
        struct Valley
        {
            Repr sol;
            double cost;
        };
        auto cheaper = [](const Valley &a, const Valley &b)
        { return a.cost < b.cost; };

        std::vector<Valley> uneroded; // candidates to erode
        std::vector<Valley> eroded;   // exhausted

        // one local optimum, from the chosen start mode.
        Repr best = local_search(
            inst, repr_random<Repr>(inst, rng, params.start), params.gravity, rng);
        double best_cost = repr_cost(inst, best);

        int clouds_run = 0;

        for (int cloud = 0; cloud < params.max_clouds; ++cloud)
        {
            ++clouds_run;

            // Exploration phase: rain and gravity.
            for (int d = 0; d < params.rain_drops; ++d)
            {
                Repr drop = repr_random<Repr>(inst, rng, params.start);
                Repr opt = local_search(inst, drop, params.gravity, rng);
                double c = repr_cost(inst, opt);

                uneroded.push_back(Valley{opt, c});
                if (c < best_cost)
                {
                    best = opt;
                    best_cost = c;
                }
            }

            // Exploitation phase: erosion of the most promising valleys.
            std::sort(uneroded.begin(), uneroded.end(), cheaper);
            const int to_erode =
                std::min<int>(params.erode_top, static_cast<int>(uneroded.size()));

            std::vector<Valley> keep;
            for (int v = 0; v < static_cast<int>(uneroded.size()); ++v)
            {
                if (v >= to_erode)
                {
                    keep.push_back(uneroded[v]);
                    continue;
                }

                Valley current = uneroded[v];
                bool improved_overall = false;

                for (int a = 0; a < params.erosion_attempts; ++a)
                {
                    Repr deeper =
                        local_search(inst, current.sol, params.erosion, rng);
                    double dc = repr_cost(inst, deeper);

                    if (dc < current.cost)
                    {
                        current = Valley{deeper, dc};
                        improved_overall = true;
                        if (dc < best_cost)
                        {
                            best = deeper;
                            best_cost = dc;
                        }
                    }
                    else
                    {
                        break; // blocked direction
                    }
                }

                if (improved_overall)
                    keep.push_back(current);
                else
                    eroded.push_back(current); // exhausted
            }

            uneroded.swap(keep);
        }

        WaterFlowResult<Repr> result{best, repr_makespan(inst, best), best_cost,
                                     clouds_run, repr_nodes_in_cycle(inst, best)};
        return result;
    }
} // namespace jssp

#endif // JSSP_WATER_FLOW_HPP
