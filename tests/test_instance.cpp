// Unit tests for instance parsing and bound computation.
#include "doctest/doctest.h"

#include <stdexcept>
#include <utility>
#include <vector>

#include "../src/instance.hpp"

using jssp::Instance;

// The 3x3 example from the project document.
//
// job 0: m0(3) m1(3) m2(3)
// job 1: m0(2) m2(3) m1(4)
// job 2: m1(3) m0(2) m2(1)
//
// Expected bounds (per the document's definitions):
//   upper bound = sum of all durations = 3+3+3 + 2+3+4 + 2+3+1 = 24
//   lower bound = max(maxJobs, maxMachines)
//       maxJobs     = max(9, 9, 6)            = 9
//       maxMachines : m0 = 3+2+2 = 7
//                     m1 = 3+4+3 = 10
//                     m2 = 3+3+1 = 7          -> 10
//   => lower bound = max(9, 10) = 10
static Instance make_ft03()
{
    std::vector<std::vector<std::pair<int, int>>> jobs = {
        {{0, 3}, {1, 3}, {2, 3}},
        {{0, 2}, {2, 3}, {1, 4}},
        {{1, 3}, {0, 2}, {2, 1}},
    };
    return Instance::from_data(/*num_machines=*/3, jobs);
}

TEST_CASE("instance dimensions are read correctly")
{
    Instance inst = make_ft03();
    CHECK(inst.num_jobs() == 3);
    CHECK(inst.num_machines() == 3);
    CHECK(inst.num_operations() == 9);
}

TEST_CASE("operations keep machine, duration and precedence order")
{
    Instance inst = make_ft03();

    // job 1 should be: m0(2) m2(3) m1(4), inorder.
    const auto &j1 = inst.job(1);
    REQUIRE(j1.size() == 3);

    CHECK(j1[0].machine == 0);
    CHECK(j1[0].duration == 2);
    CHECK(j1[0].position == 0);

    CHECK(j1[1].machine == 2);
    CHECK(j1[1].duration == 3);
    CHECK(j1[1].position == 1);

    CHECK(j1[2].machine == 1);
    CHECK(j1[2].duration == 4);
    CHECK(j1[2].position == 2);

    // Convenience accessors must agree with the stored operations.
    CHECK(inst.machine(2, 0) == 1);
    CHECK(inst.duration(2, 0) == 3);
}

TEST_CASE("upper bound is the sum of all durations")
{
    Instance inst = make_ft03();
    CHECK(inst.upper_bound() == 24);
}

TEST_CASE("lower bound is max(maxJobs, maxMachines)")
{
    Instance inst = make_ft03();
    CHECK(inst.lower_bound() == 10);
}

TEST_CASE("a single-machine instance has coincident bounds")
{
    // everything is forced to run serially, so the schedule length is fixed.
    // maxMachines = total load = upper bound.
    //   job 0: m0(5)
    //   job 1: m0(7)
    std::vector<std::vector<std::pair<int, int>>> jobs = {
        {{0, 5}},
        {{0, 7}},
    };
    Instance inst = Instance::from_data(1, jobs);

    CHECK(inst.upper_bound() == 12); // 5 + 7
    CHECK(inst.lower_bound() == 12); // maxMachines = 12 dominates maxJobs = 7
}

TEST_CASE("from_data rejects an out-of-range machine")
{
    std::vector<std::vector<std::pair<int, int>>> jobs = {
        {{5, 3}}, // machine 5 does not exist when num_machines = 2
    };
    auto build = [&]
    { return Instance::from_data(2, jobs); };
    CHECK_THROWS_AS(build(), std::runtime_error);
}
