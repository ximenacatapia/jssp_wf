// JSSP solver driver (Water-Flow Algorithm).
//
// Just for testing at the moment it prints its basic data and bounds, and shows the solution representation:
// the identity permutation and one random permutation. Scheduling, cost and
// the Water-Flow search are added in later stages.
//
// Usage: ./jssp-wfa <instance_path>
#include <iostream>
#include <random>

#include "instance.hpp"
#include "solution.hpp"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <instance_path>\n";
        return 1;
    }

    try
    {
        jssp::Instance inst = jssp::Instance::from_file(argv[1]);

        std::cout << "== Instance ==\n";
        std::cout << "jobs (n):     " << inst.num_jobs() << "\n";
        std::cout << "machines (m): " << inst.num_machines() << "\n";
        std::cout << "operations:   " << inst.num_operations() << "\n";
        std::cout << "lower bound:  " << inst.lower_bound() << "\n";
        std::cout << "upper bound:  " << inst.upper_bound() << "\n\n";

        std::cout << "== Solution representation (operation-based) ==\n";

        jssp::Solution id = jssp::Solution::identity(inst);
        std::cout << "identity: " << id.to_string()
                  << (id.is_valid() ? "   [valid]" : "   [INVALID]") << "\n";

        std::random_device rd;
        std::mt19937 rng(rd());
        jssp::Solution rnd = jssp::Solution::random(inst, rng);
        std::cout << "random:   " << rnd.to_string()
                  << (rnd.is_valid() ? "   [valid]" : "   [INVALID]") << "\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
