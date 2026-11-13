// Unit tests for critical blocks and N5 moves (Stage 2 of the N5 neighborhood).
#include "doctest/doctest.h"

#include <random>
#include <utility>
#include <vector>

#include "../src/eval/critical.hpp"
#include "../src/eval/schedule.hpp"
#include "../src/model/instance.hpp"
#include "../src/model/solution.hpp"

using jssp::Instance;
using jssp::makespan_of;
using jssp::n5_moves;
using jssp::N5Move;
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

TEST_CASE("every N5 move yields a valid solution")
{
    Instance inst = make_ft03();
    std::mt19937 rng(1);

    for (int trial = 0; trial < 500; ++trial)
    {
        Solution s = Solution::random(inst, rng);
        std::vector<N5Move> moves = n5_moves(inst, s);
        for (const auto &mv : moves)
        {
            Solution nb = s;
            nb.swap_positions(mv.pos_a, mv.pos_b);
            CHECK(nb.is_valid()); // a swap keeps the multiset, so always valid
        }
    }
}

TEST_CASE("N5 moves swap two distinct positions")
{
    Instance inst = make_ft03();
    std::mt19937 rng(7);

    for (int trial = 0; trial < 200; ++trial)
    {
        Solution s = Solution::random(inst, rng);
        for (const auto &mv : n5_moves(inst, s))
            CHECK(mv.pos_a != mv.pos_b);
    }
}

TEST_CASE("a meaningful fraction of N5 moves improve the makespan")
{
    // N5 only touches critical-block ends, it should improve a lot more
    // than a random swap would.
    Instance inst = make_ft03();
    std::mt19937 rng(123);

    int total = 0, improved = 0;
    for (int trial = 0; trial < 1000; ++trial)
    {
        Solution s = Solution::random(inst, rng);
        int mk0 = makespan_of(inst, s);
        for (const auto &mv : n5_moves(inst, s))
        {
            ++total;
            Solution nb = s;
            nb.swap_positions(mv.pos_a, mv.pos_b);
            if (makespan_of(inst, nb) < mk0)
                ++improved;
        }
    }
    REQUIRE(total > 0);

    CHECK(improved * 3 >= total);
}
