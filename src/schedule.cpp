#include "schedule.hpp"

#include <algorithm>

namespace jssp
{

    Schedule::Schedule(const Instance &inst, const Solution &sol)
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

    int makespan_of(const Instance &inst, const Solution &sol)
    {
        return Schedule(inst, sol).makespan();
    }

} // namespace jssp
