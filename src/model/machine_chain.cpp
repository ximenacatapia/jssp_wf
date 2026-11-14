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

        // Build a feasible operation order by shuffling job ids (each job m
        // times) and reading operations in order -- the same idea as the
        // operation-based encoding, which is always feasible.
        std::vector<int> jobs;
        jobs.reserve(static_cast<size_t>(n) * m);
        for (int j = 0; j < n; ++j)
            for (int r = 0; r < m; ++r)
                jobs.push_back(j);
        std::shuffle(jobs.begin(), jobs.end(), rng);

        // Walk that order; for each job take its next pending operation and
        // append it to its machine's chain. Because we read in a feasible
        // order, each machine's chain ends up feasible too.
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

        // Start from a FEASIBLE chain and perturb it with a FEW random swaps.
        // This yields a solution that is nonfeasible but only MILDLY so: a
        // handful of cycles rather than near-total chaos. A fully random order
        // per machine (the naive approach) produces hundreds of interlocking
        // cycles that the local search cannot realistically untangle, so the
        // search would never escape.
        //
        // The number of perturbations is critical and was measured on the
        // 40x40: 3 swaps leave the chain infeasible but RELIABLY REPAIRABLE (the
        // cycle-directed neighborhood slides it back to a feasible optimum on
        // every seed tested), ~5 is repairable most of the time, and 10+ swaps
        // interlock into hundreds of cycles the search cannot escape. The old
        // value (2*m = 80 here) was far too destructive: it left ~1450 of 1600
        // operations in cycles, so the run never became feasible. We keep it
        // small and absolute (not scaled by m) on purpose.
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
        // The machine of an operation is fixed by the instance: op_id = job*m +
        // pos, and the instance says which machine processes it.
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
        // Return the cached result if the chain has not changed since last time.
        if (valid_)
            return cache_;

        const int m = inst_->num_machines();
        const int total = inst_->num_operations();

        // --- Build the disjunctive graph --------------------------------------
        // Nodes are operations (ids 0..total-1). Arcs:
        //   JOB arcs:     op(job,pos) -> op(job,pos+1)
        //   MACHINE arcs: consecutive ops in each machine's chain
        // We store successors and in-degree for Kahn's algorithm. Each arc also
        // carries the weight = duration of its SOURCE operation, so the longest
        // path gives the makespan.
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

        // --- Kahn's algorithm + longest path ----------------------------------
        // 'finish[id]' = earliest finish time of operation id (longest path to
        // it, inclusive of its own duration). Process nodes in topological
        // order: when a node comes out of the queue, all its predecessors are
        // done, so its finish time is final.
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
                // Relax: v cannot finish before u finishes plus v's duration.
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

        // Record which operations are stuck in cycles: those whose in-degree
        // never reached 0 (they were never dequeued by Kahn).
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