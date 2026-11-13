#include "critical.hpp"

namespace jssp
{

    namespace
    {
        std::vector<int> positions_of_ops(const Instance &inst,
                                          const Solution &sol)
        {
            const int m = inst.num_machines();
            std::vector<int> pos_of_op(inst.num_operations(), -1);
            std::vector<int> seen(inst.num_jobs(), 0); // occurrences per job so far

            const auto &seq = sol.sequence();
            for (int i = 0; i < static_cast<int>(seq.size()); ++i)
            {
                int job = seq[i];
                int k = seen[job]++;
                int op_id = job * m + k;
                pos_of_op[op_id] = i;
            }
            return pos_of_op;
        }
    } // namespace

    std::vector<N5Move> n5_moves(const Instance &inst, const Solution &sol)
    {
        const int m = inst.num_machines();
        std::vector<N5Move> moves;

        Schedule sched(inst, sol);
        std::vector<int> path = sched.critical_path();
        if (path.size() < 2)
            return moves;

        std::vector<int> pos_of_op = positions_of_ops(inst, sol);

        auto machine_of = [&](int op_id)
        { return inst.machine(op_id / m, op_id % m); };

        std::vector<std::pair<int, int>> blocks;
        int start = 0;
        for (int i = 1; i <= static_cast<int>(path.size()); ++i)
        {
            if (i == static_cast<int>(path.size()) ||
                machine_of(path[i]) != machine_of(path[start]))
            {
                blocks.push_back({start, i - 1});
                start = i;
            }
        }

        const int num_blocks = static_cast<int>(blocks.size());
        for (int b = 0; b < num_blocks; ++b)
        {
            int first = blocks[b].first;
            int last = blocks[b].second;
            int size = last - first + 1;
            if (size < 2)
                continue;

            bool is_first_block = (b == 0);
            bool is_last_block = (b == num_blocks - 1);

            if (!is_first_block)
            {
                int a = pos_of_op[path[first]];
                int c = pos_of_op[path[first + 1]];
                moves.push_back(N5Move{a, c});
            }

            if (!is_last_block && !(size == 2 && !is_first_block))
            {
                int a = pos_of_op[path[last - 1]];
                int c = pos_of_op[path[last]];
                moves.push_back(N5Move{a, c});
            }
        }

        return moves;
    }

} // namespace jssp
