#include "solution.hpp"

#include <algorithm>
#include <sstream>

namespace jssp
{

    Solution::Solution(const Instance &inst, std::vector<int> sequence)
        : inst_(&inst), seq_(std::move(sequence)) {}

    Solution Solution::identity(const Instance &inst)
    {
        const int n = inst.num_jobs();
        const int m = inst.num_machines();

        std::vector<int> seq;
        seq.reserve(static_cast<size_t>(n) * m);
        for (int job = 0; job < n; ++job)
        {
            for (int rep = 0; rep < m; ++rep)
            {
                seq.push_back(job);
            }
        }
        return Solution(inst, std::move(seq));
    }

    Solution Solution::random(const Instance &inst, std::mt19937 &rng)
    {
        Solution s = identity(inst);
        std::shuffle(s.seq_.begin(), s.seq_.end(), rng);
        return s;
    }

    void Solution::swap_positions(int a, int b)
    {
        std::swap(seq_[a], seq_[b]);
    }

    bool Solution::is_valid() const
    {
        const int n = inst_->num_jobs();
        const int m = inst_->num_machines();

        // Length must be exactly n*m.
        if (static_cast<int>(seq_.size()) != n * m)
        {
            return false;
        }

        // Count occurrences of each job id; each must appear exactly m times, and
        // no id may fall outside [0, n).
        std::vector<int> count(n, 0);
        for (int job : seq_)
        {
            if (job < 0 || job >= n)
            {
                return false;
            }
            ++count[job];
        }
        for (int job = 0; job < n; ++job)
        {
            if (count[job] != m)
            {
                return false;
            }
        }
        return true;
    }

    std::string Solution::to_string() const
    {
        std::ostringstream out;
        for (size_t i = 0; i < seq_.size(); ++i)
        {
            if (i > 0)
                out << ' ';
            out << seq_[i];
        }
        return out.str();
    }

} // namespace jssp
