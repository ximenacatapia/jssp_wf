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

    MachineChain MachineChain::random_nonfeasible(const Instance &inst, std::mt19937 &rng)
    {
        const int n = inst.num_jobs();
        (void)n;
        const int m = inst.num_machines();

        // Start from a feasible chain and perturb it with a FEW random swaps.
        MachineChain chain = random_feasible(inst, rng);

        std::uniform_int_distribution<int> pick_machine(0, m - 1);
        int perturbations = 3;
        for (int t = 0; t < perturbations; ++t)
        {
            int k = pick_machine(rng);
            int len = static_cast<int>(chain.order()[k].size());
            if (len < 2)
                continue;
            std::uniform_int_distribution<int> pick_pos(0, len - 1);
            int a = pick_pos(rng), b = pick_pos(rng);
            if (a != b)
                chain.swap_in_machine(k, a, b);
        }
        return chain;
    }

    void MachineChain::swap_adjacent(int machine, int pos)
    {
        std::swap(order_[machine][pos], order_[machine][pos + 1]);
        invalidate();
    }

    void MachineChain::swap_in_machine(int machine, int a, int b)
    {
        std::swap(order_[machine][a], order_[machine][b]);
        invalidate();
    }

    int MachineChain::machine_of(int op_id) const
    {
        const int m = inst_->num_machines();
        int job = op_id / m, pos = op_id % m;
        return inst_->machine(job, pos);
    }

    int MachineChain::position_in_machine(int op_id) const
    {
        int mk = machine_of(op_id);
        if (mk < 0)
            return -1;
        const auto &ops = order_[mk];
        for (int i = 0; i < static_cast<int>(ops.size()); ++i)
            if (ops[i] == op_id)
                return i;
        return -1;
    }

    const ChainEvaluation &MachineChain::evaluate() const
    {
        if (valid_)
            return cache_;

        const int m = inst_->num_machines();
        const int total = inst_->num_operations();

        std::vector<std::vector<int>> succ(total);
        std::vector<int> indeg(total, 0);

        auto duration_of = [&](int id)
        {
            int job = id / m, pos = id % m;
            return inst_->duration(job, pos);
        };

        // JOB arcs.
        for (int job = 0; job < inst_->num_jobs(); ++job)
            for (int pos = 0; pos + 1 < m; ++pos)
            {
                int from = job * m + pos;
                int to = job * m + (pos + 1);
                succ[from].push_back(to);
                ++indeg[to];
            }

        // MACHINE arcs.
        for (int k = 0; k < m; ++k)
            for (size_t i = 0; i + 1 < order_[k].size(); ++i)
            {
                int from = order_[k][i];
                int to = order_[k][i + 1];
                succ[from].push_back(to);
                ++indeg[to];
            }

        // Kahn's algorithm + longest path
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
        ev.nodes_in_cycle = total - processed; // nodes never dequeued are in cycles
        ev.feasible = (ev.nodes_in_cycle == 0);
        ev.makespan = static_cast<int>(makespan);

        if (ev.feasible)
            ev.topo_order = std::move(queue);

        if (!ev.feasible)
        {
            ev.cycle_ops.reserve(ev.nodes_in_cycle);
            for (int id = 0; id < total; ++id)
                if (indeg[id] > 0)
                    ev.cycle_ops.push_back(id);
        }

        // Store in the cache and return a reference to it.
        cache_ = std::move(ev);
        valid_ = true;
        return cache_;
    }

    MachineChain MachineChain::from_sequence(const Instance &inst,
                                             const std::vector<int> &seq)
    {
        // Same construction as random_feasible, but following the given job
        // sequence instead of a random shuffle
        const int n = inst.num_jobs();
        const int m = inst.num_machines();
        std::vector<int> next_pos(n, 0);
        std::vector<std::vector<int>> order(m);
        for (int j : seq)
        {
            int pos = next_pos[j]++;
            int mach = inst.machine(j, pos);
            order[mach].push_back(j * m + pos);
        }
        return MachineChain(inst, std::move(order));
    }

    std::vector<int> MachineChain::to_sequence() const
    {
        const ChainEvaluation &ev = evaluate();
        if (!ev.feasible)
            return {};

        const int m = inst_->num_machines();
        std::vector<int> seq;
        seq.reserve(ev.topo_order.size());
        for (int op : ev.topo_order)
            seq.push_back(op / m); // job id
        return seq;
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