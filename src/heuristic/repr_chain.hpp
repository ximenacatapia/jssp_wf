#ifndef JSSP_REPR_CHAIN_HPP
#define JSSP_REPR_CHAIN_HPP

#include <random>

#include "../eval/chain_cost.hpp"
#include "../eval/eval_counter.hpp"
#include "../model/instance.hpp"
#include "../model/machine_chain.hpp"
#include "representation.hpp"

/*
    Representation adapter: the machine chain (MachineChain).
    Unlike the permutation, this representation can be non-feasible, so:
*/

namespace jssp
{
    // Create a random chain, feasible or non-feasible depending on the mode.
    template <>
    inline MachineChain repr_random<MachineChain>(const Instance &inst,
                                                  std::mt19937 &rng,
                                                  StartMode mode)
    {
        if (mode == StartMode::NONFEASIBLE)
            return MachineChain::random_nonfeasible(inst, rng);
        return MachineChain::random_feasible(inst, rng);
    }

    // swap two adjacent operations within a randomly chosen machine
    // that has at least two operations, wehn we have a neighbor
    inline MachineChain repr_neighbor(const Instance &inst,
                                      const MachineChain &chain,
                                      std::mt19937 &rng)
    {
        (void)inst;
        MachineChain neighbor = chain; // copy
        const int m = neighbor.num_machines();

        const ChainEvaluation &ev = chain.evaluate();

        // non-feasible: target an operation in a cycle
        if (!ev.feasible && !ev.cycle_ops.empty())
        {
            // Pick a random operation that is stuck in a cycle.
            std::uniform_int_distribution<int> pick_op(
                0, static_cast<int>(ev.cycle_ops.size()) - 1);
            int op_id = ev.cycle_ops[pick_op(rng)];

            // Find it in its machine's chain and move it to a random position in
            // that same machine (re-ordering it may break the cycle).
            int mk = neighbor.machine_of(op_id);
            int at = neighbor.position_in_machine(op_id);
            const int len = static_cast<int>(neighbor.order()[mk].size());
            if (mk >= 0 && at >= 0 && len >= 2)
            {
                std::uniform_int_distribution<int> pick_pos(0, len - 1);
                int to = pick_pos(rng);
                while (to == at)
                    to = pick_pos(rng);
                neighbor.swap_in_machine(mk, at, to);
                return neighbor;
            }
        }

        // feasible: random adjacent sawp
        std::uniform_int_distribution<int> pick_machine(0, m - 1);
        int k = pick_machine(rng);
        for (int tries = 0; tries < m; ++tries)
        {
            if (neighbor.order()[k].size() >= 2)
                break;
            k = (k + 1) % m;
        }
        const int len = static_cast<int>(neighbor.order()[k].size());
        if (len < 2)
            return neighbor;

        std::uniform_int_distribution<int> pick_pos(0, len - 2);
        int pos = pick_pos(rng);
        neighbor.swap_adjacent(k, pos);
        return neighbor;
    }

    // Cost with non-feasibility penalty.
    inline double repr_cost(const Instance &inst, const MachineChain &chain)
    {
        bump_eval_count();
        return chain_cost(inst, chain);
    }

    inline int repr_makespan(const Instance &inst, const MachineChain &chain)
    {
        (void)inst;
        return chain.evaluate().makespan;
    }

    inline int repr_nodes_in_cycle(const Instance &inst,
                                   const MachineChain &chain)
    {
        (void)inst;
        return chain.evaluate().nodes_in_cycle;
    }
} // namespace jssp

#endif // JSSP_REPR_CHAIN_HPP
