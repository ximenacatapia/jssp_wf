// Unit tests for the operation-based permutation (the "drop").
#include "doctest/doctest.h"

#include <random>
#include <utility>
#include <vector>

#include "../src/instance.hpp"
#include "../src/solution.hpp"

using jssp::Instance;
using jssp::Solution;

// Small 3x3 instance built in memory
static Instance make_ft03()
{
    std::vector<std::vector<std::pair<int, int>>> jobs = {
        {{0, 3}, {1, 3}, {2, 3}},
        {{0, 2}, {2, 3}, {1, 4}},
        {{1, 3}, {0, 2}, {2, 1}},
    };
    return Instance::from_data(3, jobs);
}

TEST_CASE("identity has each job id exactly m times, in blocks")
{
    Instance inst = make_ft03();
    Solution s = Solution::identity(inst);

    REQUIRE(s.size() == 9); // n*m = 3*3
    CHECK(s.is_valid());

    // Expected layout: 0 0 0 1 1 1 2 2 2
    std::vector<int> expected = {0, 0, 0, 1, 1, 1, 2, 2, 2};
    CHECK(s.sequence() == expected);
}

TEST_CASE("a random solution is always valid")
{
    Instance inst = make_ft03();
    std::mt19937 rng(12345);

    // Many draws: every one must satisfy the invariant.
    for (int trial = 0; trial < 100; ++trial)
    {
        Solution s = Solution::random(inst, rng);
        CHECK(s.size() == 9);
        CHECK(s.is_valid());
    }
}

TEST_CASE("swapping two positions preserves validity")
{
    Instance inst = make_ft03();
    Solution s = Solution::identity(inst);

    s.swap_positions(0, 8); // swap first and last
    CHECK(s.is_valid());    // a swap only permutes, so counts are unchanged

    s.swap_positions(3, 3); // swapping a position with itself is a no-op
    CHECK(s.is_valid());
}

TEST_CASE("is_valid rejects a malformed sequence")
{
    Instance inst = make_ft03();

    std::vector<int> bad = {0, 0, 1, 1, 1, 2, 2, 2, 2};
    Solution s(inst, bad);
    CHECK_FALSE(s.is_valid());
}

TEST_CASE("is_valid rejects a sequence of the wrong length")
{
    Instance inst = make_ft03();
    std::vector<int> too_short = {0, 1, 2};
    Solution s(inst, too_short);
    CHECK_FALSE(s.is_valid());
}

TEST_CASE("to_string renders ids separated by spaces")
{
    Instance inst = make_ft03();
    Solution s(inst, std::vector<int>{0, 1, 2, 0, 1, 1, 2, 0, 2});
    CHECK(s.to_string() == "0 1 2 0 1 1 2 0 2");
}
