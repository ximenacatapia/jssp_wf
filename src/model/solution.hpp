#ifndef JSSP_SOLUTION_HPP
#define JSSP_SOLUTION_HPP

#include <cstdint>
#include <random>
#include <string>
#include <vector>

#include "instance.hpp"

/*
Solution representation: an operation-based permutation with repetition.

A solution is a list of n*m integers (job identifiers). Each job id appears
exactly m times. The k-th occurrence of job i (scanning left to right) is
the k-th operation of job i.
*/

namespace jssp
{

    class Solution
    {
    public:
        static Solution identity(const Instance &inst);

        static Solution random(const Instance &inst, std::mt19937 &rng);

        Solution(const Instance &inst, std::vector<int> sequence);

        const std::vector<int> &sequence() const { return seq_; }
        int size() const { return static_cast<int>(seq_.size()); }
        int operator[](int i) const { return seq_[i]; }

        void swap_positions(int a, int b);

        bool is_valid() const;

        std::string to_string() const;

    private:
        const Instance *inst_;
        std::vector<int> seq_;
    };

}

#endif // JSSP_SOLUTION_HPP
