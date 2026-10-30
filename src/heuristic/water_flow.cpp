#include "water_flow.hpp"

#include <algorithm>
#include <vector>

#include "../cost.hpp"
#include "../schedule.hpp"

namespace jssp
{

    namespace
    {

        struct Valley
        {
            Solution sol;
            double cost;
        };

        bool cheaper(const Valley &a, const Valley &b) { return a.cost < b.cost; }

    } // namespace

    WaterFlowResult water_flow(const Instance &inst,
                               const WaterFlowParams &params,
                               std::mt19937 &rng)
    {
        // Memory lists
        std::vector<Valley> uneroded; // candidates to erode
        std::vector<Valley> eroded;   // exhausted

        // best (P0)
        Solution best = local_search(inst, Solution::random(inst, rng),
                                     params.gravity, rng);
        double best_cost = cost(inst, best);

        int clouds_run = 0;

        for (int cloud = 0; cloud < params.max_clouds; ++cloud)
        {
            ++clouds_run;

            // Exploration: rain & gravity
            // Generate a population of random drops and slide each one down to a
            // local optimum, andollect those as candidate valleys.
            for (int d = 0; d < params.rain_drops; ++d)
            {
                Solution drop = Solution::random(inst, rng);
                Solution opt = local_search(inst, drop, params.gravity, rng);
                double c = cost(inst, opt);

                uneroded.push_back(Valley{opt, c});
                if (c < best_cost)
                {
                    best = opt;
                    best_cost = c;
                }
            }

            // Exploitation: erosion
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

                // intensive local search, repeated
                Valley current = uneroded[v];
                bool improved_overall = false;

                for (int a = 0; a < params.erosion_attempts; ++a)
                {
                    Solution deeper =
                        local_search(inst, current.sol, params.erosion, rng);
                    double dc = cost(inst, deeper);

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
                        break; // this direction is blocked
                    }
                }

                if (improved_overall)
                {
                    keep.push_back(current);
                }
                else
                {
                    // Exhausted (rememebr so we dont redoit)
                    eroded.push_back(current);
                }
            }

            uneroded.swap(keep);
        }

        // Build the result with the best solution directly
        return WaterFlowResult{best, makespan_of(inst, best), best_cost,
                               clouds_run, 0};
    }

} // namespace jssp
