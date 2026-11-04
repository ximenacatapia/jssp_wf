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
#include <fstream>

#include "model/instance.hpp"
#include "model/solution.hpp"
#include "model/machine_chain.hpp"
#include "eval/schedule.hpp"
#include "eval/cost.hpp"
#include "eval/chain_cost.hpp"
#include "heuristic/repr_permutation.hpp"
#include "heuristic/repr_chain.hpp"
#include "heuristic/water_flow.hpp"
#include "io/solution_writer.hpp"
#include "io/solution_checker.hpp"

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

    // True iff a file exists at 'path'.
    bool file_exists(const std::string &path)
    {
        struct stat st;
        return stat(path.c_str(), &st) == 0;
    }

    // so the files don't overwrite eachother
    std::string next_solution_path(const std::string &stem)
    {
        for (int i = 1;; ++i)
        {
            std::string candidate =
                "results/" + stem + "-" + std::to_string(i) + ".sol.txt";
            if (!file_exists(candidate))
                return candidate;
        }
    }

    // Reads a "--key value" flag from the arguments
    long long flag(int argc, char **argv, const std::string &key,
                   long long fallback)
    {
        for (int i = 2; i + 1 < argc; ++i)
        {
            if (argv[i] == key)
                return std::stoll(argv[i + 1]);
        }
        return fallback;
    }

    // Reads a "--key value" string flag. Returns "" if absent.
    std::string str_flag(int argc, char **argv, const std::string &key)
    {
        for (int i = 2; i + 1 < argc; ++i)
        {
            if (argv[i] == key)
                return argv[i + 1];
        }
        return "";
    }

    // Writing the solution file
    void write_result_file(const jssp::Instance &inst,
                           const jssp::Solution &best,
                           const std::string &path)
    {
        jssp::write_solution(inst, best, path);
    }

    void write_result_file(const jssp::Instance &inst,
                           const jssp::MachineChain &best,
                           const std::string &path)
    {
        std::ofstream out(path);
        out << best.to_string();
        jssp::ChainEvaluation ev = best.evaluate();
        out << "makespan: " << ev.makespan << "\n";
        out << "ciclos: " << ev.nodes_in_cycle << "\n";
        out << "Cota Sup: " << inst.upper_bound() << "\n";
        out << "Cota Inf: " << inst.lower_bound() << "\n";
        out << "Costo: " << jssp::chain_cost(inst, best) << "\n";
    }

    // Run + report + write  (generic over the representation)
    template <class Repr>
    int run_and_report(const jssp::Instance &inst,
                       const jssp::WaterFlowParams &params,
                       std::mt19937 &rng,
                       const std::string &instance_path)
    {
        const auto t0 = std::chrono::steady_clock::now();
        jssp::WaterFlowResult<Repr> r = jssp::water_flow<Repr>(inst, params, rng);
        const auto t1 = std::chrono::steady_clock::now();
        const double seconds = std::chrono::duration<double>(t1 - t0).count();

        std::cout << "== Water-Flow result ==\n";
        std::cout << "feasible:   " << (jssp::is_feasible(r) ? "YES" : "NO")
                  << "\n";
        std::cout << "iterations: " << r.iterations << "\n";
        std::cout << "cost:       " << r.cost << "\n";
        std::cout << "makespan:   " << r.makespan
                  << " (nodes in cycle: " << r.nodes_in_cycle << ")\n";
        std::cout << "time:       " << seconds << " s\n";

        mkdir("results", 0755);
        const std::string sol_path = next_solution_path(stem_of(instance_path));
        write_result_file(inst, r.best, sol_path);
        std::cout << "\nsolution written to " << sol_path << "\n";
        return 0;
    }

}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <instance_path>"
                  << " [--repr permutation|chain] [--start feasible|non-feasible]"
                  << " [--seed S] [--clouds C] [--drops D] [--erode-top T]"
                  << " [--erode-attempts A] [--gravity-sample G]"
                  << " [--erosion-sample E] [--check FILE]\n";
        return 1;
    }

    try
    {
        const std::string instance_path = argv[1];
        jssp::Instance inst = jssp::Instance::from_file(instance_path);

        // Check mode -----------
        const std::string check_path = str_flag(argc, argv, "--check");
        if (!check_path.empty())
        {
            jssp::CheckResult cr =
                jssp::check_solution_file(inst, check_path);

            std::cout << "== Check: " << check_path << " ==\n";
            std::cout << "operations read: " << cr.operations
                      << " (instance has " << inst.num_operations() << ")\n";
            std::cout << "feasible:        "
                      << (cr.feasible ? "yes" : "NO") << "\n";
            if (cr.feasible)
            {
                std::cout << "makespan:        " << cr.makespan << "\n";
                std::cout << "lower bound:     " << inst.lower_bound() << "\n";
                std::cout << "upper bound:     " << inst.upper_bound() << "\n";
            }
            std::cout << "detail:          " << cr.message << "\n";
            return cr.feasible ? 0 : 1;
        }

        // Seed: from the command line if given --seed, otherwise from the system
        std::mt19937::result_type seed;
        if (long long s = flag(argc, argv, "--seed", -1); s >= 0)
        {
            seed = static_cast<std::mt19937::result_type>(s);
        }
        else
        {
            std::random_device rd;
            seed = rd();
        }
        std::mt19937 rng(seed);

        // Parameters -------
        jssp::WaterFlowParams params;
        params.max_clouds =
            static_cast<int>(flag(argc, argv, "--clouds", params.max_clouds));
        params.rain_drops =
            static_cast<int>(flag(argc, argv, "--drops", params.rain_drops));
        params.erode_top =
            static_cast<int>(flag(argc, argv, "--erode-top", params.erode_top));
        params.erosion_attempts = static_cast<int>(
            flag(argc, argv, "--erode-attempts", params.erosion_attempts));
        params.gravity.sample_size = static_cast<int>(
            flag(argc, argv, "--gravity-sample", params.gravity.sample_size));
        params.erosion.sample_size = static_cast<int>(
            flag(argc, argv, "--erosion-sample", params.erosion.sample_size));

        // Start mode (for the chain representation).
        std::string start = str_flag(argc, argv, "--start");
        if (start == "feasible")
            params.start = jssp::StartMode::FEASIBLE;
        else
            params.start = jssp::StartMode::NONFEASIBLE; // default

        std::cout << "== Instance ==\n";
        std::cout << "jobs (n):     " << inst.num_jobs() << "\n";
        std::cout << "machines (m): " << inst.num_machines() << "\n";
        std::cout << "operations:   " << inst.num_operations() << "\n";
        std::cout << "lower bound:  " << inst.lower_bound() << "\n";
        std::cout << "upper bound:  " << inst.upper_bound() << "\n";
        std::cout << "seed:         " << seed << "\n\n";

        // Representation --------
        std::string repr = str_flag(argc, argv, "--repr");
        if (repr == "permutation")
        {
            std::cout << "repr:         permutation\n\n";
            return run_and_report<jssp::Solution>(inst, params, rng, instance_path);
        }
        else
        {
            std::cout << "repr:         machine chain ("
                      << (params.start == jssp::StartMode::NONFEASIBLE
                              ? "non-feasible"
                              : "feasible")
                      << ")\n\n";
            return run_and_report<jssp::MachineChain>(inst, params, rng,
                                                      instance_path);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
