// JSSP solver driver (Water-Flow Algorithm).
//
// Just for testing at the moment it prints its basic data and bounds, and shows the solution representation:
// the identity permutation and one random permutation. Scheduling, cost and
// the Water-Flow search are added in later stages.
//
// Usage: ./jssp-wfa <instance_path>
#include <iostream>
#include <random>
#include <string>
#include <cstdint>
#include <chrono>
#include <sys/stat.h>

#include "instance.hpp"
#include "solution.hpp"
#include "schedule.hpp"
#include "cost.hpp"
#include "heuristic/local_search.hpp"
#include "heuristic/water_flow.hpp"
#include "solution_writer.hpp"

namespace
{

    // Returns the file name without directory or extension.
    std::string stem_of(const std::string &path)
    {
        size_t slash = path.find_last_of("/\\");
        std::string name = (slash == std::string::npos) ? path
                                                        : path.substr(slash + 1);
        size_t dot = name.find_last_of('.');
        if (dot != std::string::npos)
            name = name.substr(0, dot);
        return name;
    }

}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <instance_path>\n";
        return 1;
    }

    try
    {
        const std::string instance_path = argv[1];
        jssp::Instance inst = jssp::Instance::from_file(instance_path);

        // Seed: from the command line if given, otherwise from the system
        std::mt19937::result_type seed;
        if (argc >= 3)
        {
            seed = static_cast<std::mt19937::result_type>(
                std::stoul(argv[2]));
        }
        else
        {
            std::random_device rd;
            seed = rd();
        }
        std::mt19937 rng(seed);

        std::cout << "== Instance ==\n";
        std::cout << "jobs (n):     " << inst.num_jobs() << "\n";
        std::cout << "machines (m): " << inst.num_machines() << "\n";
        std::cout << "operations:   " << inst.num_operations() << "\n";
        std::cout << "lower bound:  " << inst.lower_bound() << "\n";
        std::cout << "upper bound:  " << inst.upper_bound() << "\n";
        std::cout << "seed:         " << seed << "\n\n";

        // Run the Water-Flow algorithm
        jssp::WaterFlowParams params; // default parameters

        const auto t0 = std::chrono::steady_clock::now();
        jssp::WaterFlowResult r = jssp::water_flow(inst, params, rng);
        const auto t1 = std::chrono::steady_clock::now();
        const double seconds = std::chrono::duration<double>(t1 - t0).count();

        const bool feasible = jssp::is_feasible(r);

        // report format
        std::cout << "== Water-Flow result ==\n";
        std::cout << "feasible:   " << (feasible ? "yes" : "NO") << "\n";
        std::cout << "iterations: " << r.iterations << "\n";
        std::cout << "cost:       " << r.cost << "\n";
        std::cout << "makespan:   " << r.makespan
                  << " (nodes in cycle: " << r.nodes_in_cycle << ")\n";
        std::cout << "time:       " << seconds << " s\n";

        // Write the solution file (machine-chain format)
        mkdir("results", 0755);
        const std::string sol_path =
            "results/" + stem_of(instance_path) + ".sol.txt";
        jssp::write_solution(inst, r.best, sol_path);
        std::cout << "\nsolution written to " << sol_path << "\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
