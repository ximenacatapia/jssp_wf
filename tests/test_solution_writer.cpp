// Unit tests for the solution writer.
#include "doctest/doctest.h"

#include <cctype>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "../src/instance.hpp"
#include "../src/solution.hpp"
#include "../src/solution_writer.hpp"

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

TEST_CASE("solution file has one line per machine plus the footer")
{
    Instance inst = make_ft03();
    Solution id = Solution::identity(inst);

    const std::string path = "test_sol_out.tmp";
    jssp::write_solution(inst, id, path);

    std::ifstream in(path);
    REQUIRE(in.good());

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(in, line))
        lines.push_back(line);
    in.close();
    std::remove(path.c_str());

    // 3 machine lines + 5 footer lines (makespan, ciclos, Cota Sup/Inf, Costo).
    REQUIRE(lines.size() == 8);

    // The three machine lines start with "[ ".
    for (int k = 0; k < 3; ++k)
    {
        CHECK(lines[k].rfind("[ ", 0) == 0);
    }

    // Footer content.
    CHECK(lines[3] == "makespan: 22"); // identity makespan (from Stage 3)
    CHECK(lines[4] == "ciclos: 0");
    CHECK(lines[5] == "Cota Sup: 24");
    CHECK(lines[6] == "Cota Inf: 10");
    CHECK(lines[7].rfind("Costo: ", 0) == 0);
}

TEST_CASE("every operation appears exactly once across all machines")
{
    Instance inst = make_ft03();
    std::mt19937 rng(5);
    Solution s = Solution::random(inst, rng);

    const std::string path = "test_sol_out2.tmp";
    jssp::write_solution(inst, s, path);

    std::ifstream in(path);
    std::string content((std::istreambuf_iterator<char>(in)),
                        std::istreambuf_iterator<char>());
    in.close();
    std::remove(path.c_str());

    // Count the "[job, op, dur]" triples: there must be exactly n*m of them.
    // Each inner triple contains exactly two commas inside the brackets, so we
    // count occurrences of "], [" plus the ends. Simpler: count '[' that are
    // followed by a digit (operation brackets), which equals n*m.
    int op_brackets = 0;
    for (size_t i = 0; i + 1 < content.size(); ++i)
    {
        if (content[i] == '[' && (std::isdigit(content[i + 1])))
        {
            ++op_brackets;
        }
    }
    CHECK(op_brackets == inst.num_operations()); // 9
}
