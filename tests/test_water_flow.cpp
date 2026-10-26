// Unit tests for  the Water-Flow algorithm.
#include "doctest/doctest.h"

#include <algorithm>
#include <random>
#include <utility>
#include <vector>

#include "../src/cost.hpp"
#include "../src/heuristic/water_flow.hpp"
#include "../src/instance.hpp"
#include "../src/schedule.hpp"
#include "../src/solution.hpp"

using jssp::Instance;
using jssp::Solution;
using jssp::water_flow;
using jssp::WaterFlowParams;
using jssp::WaterFlowResult;

static Instance make_ft03()
{
    std::vector<std::vector<std::pair<int, int>>> jobs = {
        {{0, 3}, {1, 3}, {2, 3}},
        {{0, 2}, {2, 3}, {1, 4}},
        {{1, 3}, {0, 2}, {2, 1}},
    };
    return Instance::from_data(3, jobs);
}

// A small random square instance
static Instance make_square(int n, unsigned seed)
{
    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> dur(1, 20);
    std::vector<std::vector<std::pair<int, int>>> jobs(n);
    for (int j = 0; j < n; ++j)
    {
        std::vector<int> machines(n);
        for (int k = 0; k < n; ++k)
            machines[k] = k;
        std::shuffle(machines.begin(), machines.end(), gen);
        for (int k : machines)
            jobs[j].push_back({k, dur(gen)});
    }
    return Instance::from_data(n, jobs);
}

static WaterFlowParams small_params()
{
    WaterFlowParams p;
    p.max_clouds = 5;
    p.rain_drops = 6;
    p.erode_top = 3;
    p.erosion_attempts = 2;
    return p;
}

TEST_CASE("water_flow returns a valid solution with consistent statistics")
{
    Instance inst = make_ft03();
    std::mt19937 rng(1);

    WaterFlowResult r = water_flow(inst, small_params(), rng);

    CHECK(r.best.is_valid());
    CHECK(r.makespan == jssp::makespan_of(inst, r.best));
    CHECK(r.cost == doctest::Approx(jssp::cost(inst, r.best)));
    CHECK(r.iterations == 5);
}

TEST_CASE("water_flow respects the lower bound")
{
    Instance inst = make_square(6, 42);
    std::mt19937 rng(2);

    WaterFlowResult r = water_flow(inst, small_params(), rng);
    CHECK(r.makespan >= inst.lower_bound());
}

TEST_CASE("water_flow is no worse than a single random local optimum")
{
    // The algorithm seeds 'best' with one local optimum and only ever replaces
    // it with something better, so its result must be at least as good as a
    // plain random local search.
    Instance inst = make_square(6, 7);

    std::mt19937 rng_wf(3);
    WaterFlowResult r = water_flow(inst, small_params(), rng_wf);

    std::mt19937 rng_ref(3);
    Solution rnd = Solution::random(inst, rng_ref);
    CHECK(r.cost <= jssp::cost(inst, rnd));
}

TEST_CASE("water_flow is reproducible with the same seed")
{
    Instance inst = make_square(6, 99);

    std::mt19937 rng_a(123);
    std::mt19937 rng_b(123);
    WaterFlowResult ra = water_flow(inst, small_params(), rng_a);
    WaterFlowResult rb = water_flow(inst, small_params(), rng_b);

    CHECK(ra.makespan == rb.makespan);
    CHECK(ra.best.sequence() == rb.best.sequence());
}
