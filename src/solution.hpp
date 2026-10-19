#ifndef JSSP_SOLUTION_HPP
#define JSSP_SOLUTION_HPP

#include <cstdint>
#include <random>
#include <string>
#include <vector>

#include "instance.hpp"

// ============================================================================
//  Solution representation: an operation-based permutation with repetition.
// ----------------------------------------------------------------------------
//  A solution is a list of n*m integers (job identifiers). Each job id appears
//  exactly m times. The k-th occurrence of job i (scanning left to right) is
//  the k-th operation of job i.
//
//  Example for the 3x3 instance (n = m = 3), a list of 9 ids:
//  [0, 1, 2, 0, 1, 1, 2, 0, 2]
//  This encoding is always decodable into a FEASIBLE schedule: when the k-th
//  operation of a job is placed, its k-1 earlier operations already appeared
//  earlier in the list, so intra-job precedence holds by construction. No cycle
//  can form in the disjunctive graph, which is why this project does not need
//  the infeasibility penalty described in the JSSP document.
//
//  In the Water-Flow algorithm this object is a "drop of water".
// ============================================================================

namespace jssp
{

    class Solution
    {
    public:
        // Builds the identity permutation for the given instance:
        //   [0, 0, ..., 1, 1, ..., n-1, n-1, ...]  (each job id m times in a row).
        // It is a valid starting point (every job appears exactly m times).
        static Solution identity(const Instance &inst);

        // Builds a uniformly random valid permutation by shuffling the identity
        // with the provided random engine.
        static Solution random(const Instance &inst, std::mt19937 &rng);

        // Wraps an existing sequence. Does NOT check validity; call is_valid() if
        // the sequence comes from an untrusted source.
        Solution(const Instance &inst, std::vector<int> sequence);

        // --- Access ------------------------------------------------------------
        const std::vector<int> &sequence() const { return seq_; }
        int size() const { return static_cast<int>(seq_.size()); }
        int operator[](int i) const { return seq_[i]; }

        // Mutable access to a position (used by the neighborhood operator later).
        void swap_positions(int a, int b);

        // --- Validity ----------------------------------------------------------
        // True iff every job id in [0, n) appears exactly m times and the sequence
        // has length n*m. This is the invariant that guarantees feasibility.
        bool is_valid() const;

        // --- Output ------------------------------------------------------------
        // Renders the sequence as space-separated job ids, e.g. "0 1 2 0 1 ...".
        std::string to_string() const;

    private:
        const Instance *inst_; // non-owning; the instance outlives the solution
        std::vector<int> seq_; // the operation-based permutation
    };

} // namespace jssp

#endif // JSSP_SOLUTION_HPP
