// Unit tests for the decoder
#include "doctest/doctest.h"

#include <algorithm>
#include <random>
#include <utility>
#include <vector>

#include "../src/instance.hpp"
#include "../src/schedule.hpp"
#include "../src/solution.hpp"

using jssp::Instance;
using jssp::Schedule;
using jssp::Solution;

static Instance make_ft03()
{
    std::vector<std::vector<std::pair<int, int>>> jobs = {
        {{0, 3}, {1, 3}, {2, 3}}, // job 0: m0(3) m1(3) m2(3)
        {{0, 2}, {2, 3}, {1, 4}}, // job 1: m0(2) m2(3) m1(4)
        {{1, 3}, {0, 2}, {2, 1}}, // job 2: m1(3) m0(2) m2(1)
    };
    return Instance::from_data(3, jobs);
}

TEST_CASE("identity permutation decodes to the hand-computed makespan")
{
    Instance inst = make_ft03();
    Solution id = Solution::identity(inst); // 0 0 0 1 1 1 2 2 2

    Schedule sched(inst, id);

    CHECK(sched.makespan() == 22);
}

TEST_CASE("every operation respects job precedence and machine exclusion")
{
    Instance inst = make_ft03();
    std::mt19937 rng(777);

    for (int trial = 0; trial < 50; ++trial)
    {
        Solution s = Solution::random(inst, rng);
        Schedule sched(inst, s);
        const auto &ops = sched.operations();

        REQUIRE(static_cast<int>(ops.size()) == inst.num_operations());

        // Job precedence
        std::vector<int> last_end_of_job(inst.num_jobs(), 0);
        std::vector<int> last_pos_of_job(inst.num_jobs(), -1);

        // Machine exclusion, in which we collect intervals per machine and check no overlap.
        std::vector<std::vector<std::pair<int, int>>> per_machine(
            inst.num_machines());

        for (const auto &op : ops)
        {
            // duration must be positive relative to the instance data.
            CHECK(op.end - op.start == inst.duration(op.job, op.position));

            // precedence
            CHECK(op.position == last_pos_of_job[op.job] + 1);
            CHECK(op.start >= last_end_of_job[op.job]);
            last_end_of_job[op.job] = op.end;
            last_pos_of_job[op.job] = op.position;

            per_machine[op.machine].push_back({op.start, op.end});
        }

        // No two intervals on the same machine overlap.
        for (auto &intervals : per_machine)
        {
            std::sort(intervals.begin(), intervals.end());
            for (size_t i = 1; i < intervals.size(); ++i)
            {
                CHECK(intervals[i].first >= intervals[i - 1].second);
            }
        }
    }
}

TEST_CASE("makespan is never below the lower bound")
{
    Instance inst = make_ft03();
    std::mt19937 rng(2024);

    for (int trial = 0; trial < 50; ++trial)
    {
        Solution s = Solution::random(inst, rng);
        int mk = jssp::makespan_of(inst, s);
        CHECK(mk >= inst.lower_bound());
    }
}

TEST_CASE("a single-machine instance forces the sum of durations")
{
    // he machine must run both operations back to back,
    // so the makespan equals the total load regardless of order.
    std::vector<std::vector<std::pair<int, int>>> jobs = {
        {{0, 5}},
        {{0, 7}},
    };
    Instance inst = Instance::from_data(1, jobs);

    Solution a(inst, std::vector<int>{0, 1});
    Solution b(inst, std::vector<int>{1, 0});
    CHECK(jssp::makespan_of(inst, a) == 12);
    CHECK(jssp::makespan_of(inst, b) == 12);
}
