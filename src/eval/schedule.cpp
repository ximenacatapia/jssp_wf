#include "schedule.hpp"

#include <algorithm>

namespace jssp
{

    Schedule::Schedule(const Instance &inst, const Solution &sol) : inst_(&inst)
    {
        const int n = inst.num_jobs();
        const int m = inst.num_machines();

        std::vector<int> next_op(n, 0);       // index of the next pending operation of job j (0..m)
        std::vector<int> job_ready(n, 0);     // time at which job j's previous operation finishes
        std::vector<int> machine_ready(m, 0); // time at which machine k becomes free

        ops_.reserve(static_cast<size_t>(n) * m);
        makespan_ = 0;

        for (int job : sol.sequence())
        {
            const int pos = next_op[job];
            const Operation &op = inst.job(job)[pos];

            // Earliest feasible start, in which both the job and the machine must be free.
            const int start = std::max(job_ready[job], machine_ready[op.machine]);
            const int end = start + op.duration;

            ops_.push_back(ScheduledOp{job, pos, op.machine, start, end});

            // Advance the clocks and the job's operation pointer.
            job_ready[job] = end;
            machine_ready[op.machine] = end;
            next_op[job] = pos + 1;

            makespan_ = std::max(makespan_, end);
        }
    }

    std::vector<int> Schedule::critical_path() const
    {
        const int m = inst_->num_machines();
        const int total = inst_->num_operations();

        // Index operations by their global id (id = job*m + pos) so we can look
        // up any operation's times directly.
        //   start_of[id], end_of[id] : its scheduled times
        //   mach_of[id]              : its machine
        std::vector<int> start_of(total), end_of(total), mach_of(total);
        for (const auto &op : ops_)
        {
            int id = op.job * m + op.position;
            start_of[id] = op.start;
            end_of[id] = op.end;
            mach_of[id] = op.machine;
        }

        // For each machine, the order in which it processed its operations, so
        // we can find the machine-predecessor of any operation.
        //   prev_on_machine[id] = the op that ran right before id on its machine
        std::vector<int> prev_on_machine(total, -1);
        {
            // Group op ids by machine, sorted by start time.
            std::vector<std::vector<int>> by_machine(m);
            for (int id = 0; id < total; ++id)
                by_machine[mach_of[id]].push_back(id);
            for (auto &ids : by_machine)
            {
                std::sort(ids.begin(), ids.end(),
                          [&](int a, int b)
                          { return start_of[a] < start_of[b]; });
                for (size_t i = 1; i < ids.size(); ++i)
                    prev_on_machine[ids[i]] = ids[i - 1];
            }
        }

        // Find the operation that finishes at the makespan: the end of a
        // critical path.
        int last = -1;
        for (int id = 0; id < total; ++id)
            if (end_of[id] == makespan_)
            {
                last = id;
                break;
            }

        // Walk backwards. At each step, the operation started exactly when its
        // critical predecessor finished (no slack). The predecessor is either
        // the previous operation of the same job, or the previous one on the
        // same machine -- whichever finished exactly at this op's start.
        std::vector<int> path;
        int u = last;
        while (u != -1)
        {
            path.push_back(u);

            int job = u / m, pos = u % m;
            int chosen = -1;

            // Job predecessor: op at (job, pos-1).
            if (pos > 0)
            {
                int jp = job * m + (pos - 1);
                if (end_of[jp] == start_of[u])
                    chosen = jp;
            }
            // Machine predecessor (if no job predecessor matched).
            if (chosen == -1)
            {
                int mp = prev_on_machine[u];
                if (mp != -1 && end_of[mp] == start_of[u])
                    chosen = mp;
            }

            u = chosen; // -1 when u starts at time 0 (path start)
        }

        std::reverse(path.begin(), path.end());
        return path;
    }

    int makespan_of(const Instance &inst, const Solution &sol)
    {
        return Schedule(inst, sol).makespan();
    }

} // namespace jssp
