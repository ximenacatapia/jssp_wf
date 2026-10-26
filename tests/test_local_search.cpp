// Unit tests for local search (gravity / hill-sliding).
#include "doctest/doctest.h"

#include <algorithm>
#include <random>
#include <utility>
#include <vector>

#include "../src/cost.hpp"
#include "../src/heuristic/local_search.hpp"
#include "../src/instance.hpp"
#include "../src/schedule.hpp"
#include "../src/solution.hpp"

using jssp::cost;
using jssp::Instance;
using jssp::local_search;
using jssp::LocalSearchParams;
using jssp::LocalSearchStrategy;
using jssp::Solution;

static Instance make_ft03()
{
    std::vector<std::vector<std::pair<int, int>>> jobs = {
        {{0, 3}, {1, 3}, {2, 3}},
        {{0, 2}, {2, 3}, {1, 4}},
        {{1, 3}, {0, 2}, {2, 1}},
    };
    return Instance::from_data(3, jobs);
}

TEST_CASE("local search never worsens the starting solution")
{
    Instance inst = make_ft03();
    std::mt19937 rng(7);

    for (auto strat : {LocalSearchStrategy::FIRST_IMPROVEMENT,
                       LocalSearchStrategy::STEEPEST})
    {
        LocalSearchParams p;
        p.strategy = strat;
        p.sample_size = 100;

        for (int trial = 0; trial < 30; ++trial)
        {
            Solution start = Solution::random(inst, rng);
            double start_cost = cost(inst, start);

            Solution local = local_search(inst, start, p, rng);

            CHECK(local.is_valid());
            CHECK(cost(inst, local) <= start_cost); // never worse
        }
    }
}

TEST_CASE("local search reaches a true local optimum on the identity start")
{
    // Starting from the (bad) identity, local search should strictly improve it
    // for this instance, since many better neighbors exist.
    Instance inst = make_ft03();
    std::mt19937 rng(123);

    LocalSearchParams p;
    p.strategy = LocalSearchStrategy::STEEPEST;
    p.sample_size = 100; // covers the small neighborhood

    Solution start = Solution::identity(inst); // makespan 22
    Solution local = local_search(inst, start, p, rng);

    CHECK(cost(inst, local) < cost(inst, start));
    CHECK(jssp::makespan_of(inst, local) < 22);
}

TEST_CASE("both strategies return valid solutions on a larger instance")
{
    // A 5x5 instance
    std::mt19937 gen(999);
    std::uniform_int_distribution<int> dur(1, 9);
    const int n = 5, m = 5;
    std::vector<std::vector<std::pair<int, int>>> jobs(n);
    for (int j = 0; j < n; ++j)
    {
        std::vector<int> machines = {0, 1, 2, 3, 4};
        std::shuffle(machines.begin(), machines.end(), gen);
        for (int k : machines)
            jobs[j].push_back({k, dur(gen)});
    }
    Instance inst = Instance::from_data(m, jobs);

    std::mt19937 rng(55);
    for (auto strat : {LocalSearchStrategy::FIRST_IMPROVEMENT,
                       LocalSearchStrategy::STEEPEST})
    {
        LocalSearchParams p;
        p.strategy = strat;
        p.sample_size = 150;

        Solution start = Solution::random(inst, rng);
        Solution local = local_search(inst, start, p, rng);

        CHECK(local.is_valid());
        CHECK(cost(inst, local) <= cost(inst, start));
    }
}
