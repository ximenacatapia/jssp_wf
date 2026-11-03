#include "machine_chain.hpp"

#include <algorithm>
#include <sstream>

namespace jssp
{

    MachineChain::MachineChain(const Instance &inst, std::vector<std::vector<int>> order)
        : inst_(&inst), order_(std::move(order)) {}

    MachineChain MachineChain::random_feasible(const Instance &inst, std::mt19937 &rng)
    {
        const int n = inst.num_jobs();
        const int m = inst.num_machines();

        // Build a feasible operation order by shuffling job ids
        std::vector<int> jobs;
        jobs.reserve(static_cast<size_t>(n) * m);
        for (int j = 0; j < n; ++j)
            for (int r = 0; r < m; ++r)
                jobs.push_back(j);
        std::shuffle(jobs.begin(), jobs.end(), rng);
        std::vector<int> next_pos(n, 0);
        std::vector<std::vector<int>> order(m);
        for (int j : jobs)
        {
            int pos = next_pos[j]++;
            int mach = inst.machine(j, pos);
            order[mach].push_back(j * m + pos); // op_id
        }
        return MachineChain(inst, std::move(order));
    }

    MachineChain MachineChain::random_infeasible(const Instance &inst, std::mt19937 &rng)
    {
        const int n = inst.num_jobs();
        const int m = inst.num_machines();

        // For each machine, collect the operations assigned to it, then shuffle
        // each machine's list independently, which gives us non feasibles
        std::vector<std::vector<int>> order(m);
        for (int j = 0; j < n; ++j)
            for (int pos = 0; pos < m; ++pos)
            {
                int mach = inst.machine(j, pos);
                order[mach].push_back(j * m + pos);
            }
        for (auto &chain : order)
            std::shuffle(chain.begin(), chain.end(), rng);

        return MachineChain(inst, std::move(order));
    }

    void MachineChain::swap_adjacent(int machine, int pos)
    {
        std::swap(order_[machine][pos], order_[machine][pos + 1]);
    }

    void MachineChain::swap_in_machine(int machine, int a, int b)
    {
        std::swap(order_[machine][a], order_[machine][b]);
    }

    ChainEvaluation MachineChain::evaluate() const
    {
        const int m = inst_->num_machines();
        const int total = inst_->num_operations();

        std::vector<std::vector<int>> succ(total);
        std::vector<int> indeg(total, 0);

        auto duration_of = [&](int id)
        {
            int job = id / m, pos = id % m;
            return inst_->duration(job, pos);
        };

        // job arcs.
        for (int job = 0; job < inst_->num_jobs(); ++job)
            for (int pos = 0; pos + 1 < m; ++pos)
            {
                int from = job * m + pos;
                int to = job * m + (pos + 1);
                succ[from].push_back(to);
                ++indeg[to];
            }

        // machine arcs.
        for (int k = 0; k < m; ++k)
            for (size_t i = 0; i + 1 < order_[k].size(); ++i)
            {
                int from = order_[k][i];
                int to = order_[k][i + 1];
                succ[from].push_back(to);
                ++indeg[to];
            }

        // kahn's algorith and longest path
        std::vector<long long> finish(total, 0);
        std::vector<int> queue;
        queue.reserve(total);

        for (int id = 0; id < total; ++id)
            if (indeg[id] == 0)
            {
                finish[id] = duration_of(id);
                queue.push_back(id);
            }

        int processed = 0;
        long long makespan = 0;
        for (size_t head = 0; head < queue.size(); ++head)
        {
            int u = queue[head];
            ++processed;
            makespan = std::max(makespan, finish[u]);

            for (int v : succ[u])
            {
                long long cand = finish[u] + duration_of(v);
                if (cand > finish[v])
                    finish[v] = cand;
                if (--indeg[v] == 0)
                    queue.push_back(v);
            }
        }

        ChainEvaluation ev;
        ev.nodes_in_cycle = total - processed;
        ev.feasible = (ev.nodes_in_cycle == 0);
        ev.makespan = static_cast<int>(makespan);
        return ev;
    }

    std::string MachineChain::to_string() const
    {
        std::ostringstream out;
        for (int k = 0; k < num_machines(); ++k)
        {
            out << "[ ";
            for (int id : order_[k])
            {
                int job = job_of(id), pos = pos_of(id);
                out << "[" << job << ", " << pos << ", "
                    << inst_->duration(job, pos) << "], ";
            }
            out << "]\n";
        }
        return out.str();
    }

} // namespace jssp
