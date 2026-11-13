// Unit tests for critical path extraction
#include "doctest/doctest.h"

#include <random>
#include <utility>
#include <vector>

#include "../src/model/instance.hpp"
#include "../src/model/solution.hpp"
#include "../src/eval/schedule.hpp"

using jssp::Instance;
using jssp::Schedule;
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

TEST_CASE("sum of critical-path durations equals the makespan")
{
    Instance inst = make_ft03();
    const int m = inst.num_machines();
    std::mt19937 rng(1);

    for (int trial = 0; trial < 300; ++trial)
    {
        Solution s = Solution::random(inst, rng);
        Schedule sched(inst, s);
        std::vector<int> path = sched.critical_path();

        int sum = 0;
        for (int id : path)
            sum += inst.duration(id / m, id % m);

        CHECK(!path.empty());
        CHECK(sum == sched.makespan());
    }
}

TEST_CASE("critical path operations are contiguous in time (no slack)")
{
    Instance inst = make_ft03();
    const int m = inst.num_machines();
    std::mt19937 rng(42);

    Solution s = Solution::random(inst, rng);
    Schedule sched(inst, s);
    std::vector<int> path = sched.critical_path();

    // Map id -> (start, end) from the schedule.
    std::vector<int> start_of(inst.num_operations());
    std::vector<int> end_of(inst.num_operations());
    for (const auto &op : sched.operations())
    {
        int id = op.job * m + op.position;
        start_of[id] = op.start;
        end_of[id] = op.end;
    }

    for (size_t i = 1; i < path.size(); ++i)
        CHECK(start_of[path[i]] == end_of[path[i - 1]]);

    CHECK(start_of[path.front()] == 0);
    CHECK(end_of[path.back()] == sched.makespan());
}
