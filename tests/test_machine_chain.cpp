// Unit tests for the machine-chain representation and its disjunctive-graph
// evaluation (Kahn + longest path + cycle detection).
#include "doctest/doctest.h"

#include <random>
#include <utility>
#include <vector>

#include "../src/model/instance.hpp"
#include "../src/model/machine_chain.hpp"

using jssp::ChainEvaluation;
using jssp::Instance;
using jssp::MachineChain;

static Instance make_ft03()
{
    std::vector<std::vector<std::pair<int, int>>> jobs = {
        {{0, 3}, {1, 3}, {2, 3}},
        {{0, 2}, {2, 3}, {1, 4}},
        {{1, 3}, {0, 2}, {2, 1}},
    };
    return Instance::from_data(3, jobs);
}

TEST_CASE("a feasible random chain is always feasible")
{
    Instance inst = make_ft03();
    std::mt19937 rng(1);

    for (int trial = 0; trial < 50; ++trial)
    {
        MachineChain mc = MachineChain::random_feasible(inst, rng);
        ChainEvaluation ev = mc.evaluate();
        CHECK(ev.feasible);
        CHECK(ev.nodes_in_cycle == 0);
        // Makespan of any feasible schedule must be at least the lower bound.
        CHECK(ev.makespan >= inst.lower_bound());
    }
}

TEST_CASE("the non-feasible-start mode does produce non-feasible chains")
{
    Instance inst = make_ft03();
    std::mt19937 rng(7);

    int nonfeasible = 0;
    for (int trial = 0; trial < 200; ++trial)
    {
        MachineChain mc = MachineChain::random_nonfeasible(inst, rng);
        ChainEvaluation ev = mc.evaluate();
        if (!ev.feasible)
        {
            ++nonfeasible;
            CHECK(ev.nodes_in_cycle > 0); // non-feasible -> some ops in cycles
        }
        else
        {
            CHECK(ev.nodes_in_cycle == 0);
        }
    }
    CHECK(nonfeasible > 0);
}

TEST_CASE("every chain lists exactly n*m operations across machines")
{
    Instance inst = make_ft03();
    std::mt19937 rng(3);

    MachineChain mc = MachineChain::random_nonfeasible(inst, rng);
    int total = 0;
    for (const auto &chain : mc.order())
        total += static_cast<int>(chain.size());
    CHECK(total == inst.num_operations()); // 9
    CHECK(mc.num_machines() == inst.num_machines());
}

TEST_CASE("a hand-built feasible chain has the expected makespan")
{

    std::vector<std::vector<int>> order = {
        {0, 3, 7}, // machine 0
        {1, 5, 6}, // machine 1
        {2, 4, 8}, // machine 2
    };
    Instance inst = make_ft03();
    MachineChain mc(inst, order);

    ChainEvaluation ev = mc.evaluate();
    CHECK(ev.feasible);
    CHECK(ev.makespan == 22);
}

TEST_CASE("a deliberately cyclic chain is detected as non-feasible")
{
    // Classic 2-job / 2-machine deadlock with crossed routes
    std::vector<std::vector<std::pair<int, int>>> jobs = {
        {{0, 3}, {1, 2}},
        {{1, 4}, {0, 1}},
    };
    Instance inst = Instance::from_data(2, jobs);

    std::vector<std::vector<int>> order = {
        {3, 0},
        {1, 2},
    };
    MachineChain mc(inst, order);
    ChainEvaluation ev = mc.evaluate();

    CHECK_FALSE(ev.feasible);
    CHECK(ev.nodes_in_cycle > 0);
}
