// Unit tests for the normalized cost function.
#include "doctest/doctest.h"

#include <utility>
#include <vector>

#include "../src/cost.hpp"
#include "../src/instance.hpp"
#include "../src/solution.hpp"

using jssp::Instance;
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

// ft03 bounds: lower = 10, upper = 24 (serial sum), range = 14.

TEST_CASE("normalize maps the lower bound to 0 and the upper bound to 1")
{
    Instance inst = make_ft03();
    CHECK(jssp::normalize_makespan(inst, 10) == doctest::Approx(0.0));
    CHECK(jssp::normalize_makespan(inst, 24) == doctest::Approx(1.0));
}

TEST_CASE("normalize is linear in between")
{
    Instance inst = make_ft03();
    CHECK(jssp::normalize_makespan(inst, 17) == doctest::Approx(0.5));
}

TEST_CASE("cost of the identity solution matches the formula")
{
    Instance inst = make_ft03();
    Solution id = Solution::identity(inst); // makespan 22
    // (22 - 10) / (24 - 10) = 12/14.
    CHECK(jssp::cost(inst, id) == doctest::Approx(12.0 / 14.0));
}

TEST_CASE("a degenerate instance with zero range yields cost 0")
{
    std::vector<std::vector<std::pair<int, int>>> jobs = {
        {{0, 5}},
    };
    Instance inst = Instance::from_data(1, jobs);
    REQUIRE(inst.lower_bound() == inst.upper_bound());

    Solution s = Solution::identity(inst);
    CHECK(jssp::cost(inst, s) == doctest::Approx(0.0));
}

TEST_CASE("lower cost means lower makespan (monotonic)")
{
    Instance inst = make_ft03();
    // A schedule with makespan 11 must cost less than one with makespan 22.
    CHECK(jssp::normalize_makespan(inst, 11) <
          jssp::normalize_makespan(inst, 22));
}
