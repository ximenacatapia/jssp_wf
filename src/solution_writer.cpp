#include "solution_writer.hpp"

#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <vector>

#include "cost.hpp"
#include "schedule.hpp"

namespace jssp
{

    void write_solution(const Instance &inst,
                        const Solution &sol,
                        const std::string &path)
    {
        std::ofstream out(path);
        if (!out)
        {
            throw std::runtime_error("Could not write solution file: " + path);
        }

        // Decode the schedule so every operation has its machine and start time.
        Schedule sched(inst, sol);

        // Group operations by machine.
        const int m = inst.num_machines();
        std::vector<std::vector<ScheduledOp>> by_machine(m);
        for (const auto &op : sched.operations())
        {
            by_machine[op.machine].push_back(op);
        }

        // Within each machine, order by the time the machine processes them
        for (auto &ops : by_machine)
        {
            std::sort(ops.begin(), ops.end(),
                      [](const ScheduledOp &a, const ScheduledOp &b)
                      {
                          if (a.start != b.start)
                              return a.start < b.start;
                          return a.end < b.end;
                      });
        }

        // One line per machine: [ [job, op, dur], [job, op, dur], ... ]
        for (int k = 0; k < m; ++k)
        {
            out << "[ ";
            for (const auto &op : by_machine[k])
            {
                const int dur = op.end - op.start;
                out << "[" << op.job << ", " << op.position << ", " << dur << "], ";
            }
            out << "]\n";
        }

        // Footer with the metrics, matching the shared format.
        const int mk = sched.makespan();
        out << "makespan: " << mk << "\n";
        out << "ciclos: 0\n"; // 0 = feasabe
        out << "Cota Sup: " << inst.upper_bound() << "\n";
        out << "Cota Inf: " << inst.lower_bound() << "\n";
        out << "Costo: " << cost(inst, sol) << "\n";
    }

} // namespace jssp
