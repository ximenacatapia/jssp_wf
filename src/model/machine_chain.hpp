#ifndef JSSP_MACHINE_CHAIN_HPP
#define JSSP_MACHINE_CHAIN_HPP

#include <random>
#include <string>
#include <vector>

#include "instance.hpp"

//  Second solution representation: the MACHINE CHAIN (cadena de maquinas).
//  Unlike the operation-based permutation (which is always feasible), this
//  representation stores, foe each machine, the order in which that machine
//  processes its operations.
//  An operation is identified by a global id:  op_id = job * m + pos
namespace jssp
{

    // Result of evaluating a machine chain against an instance.
    struct ChainEvaluation
    {
        int makespan = 0;       // longest path if feasible; partial otherwise
        int nodes_in_cycle = 0; // 0 => feasible; >0 => that many ops stuck in cycles
        bool feasible = false;
    };

    class MachineChain
    {
    public:
        // order_[k] = list of operation ids processed by machine k, in order.
        // Built so that each machine lists exactly the operations assigned to it.

        // A feasible random chain: shuffle the JOBS (as in the operation-based
        // encoding) and read off a feasible order, then group by machine. This
        // always yields a feasible chain.
        static MachineChain random_feasible(const Instance &inst, std::mt19937 &rng);

        // An (often) infeasible random chain: for each machine, put its
        // operations in a random order, independently. The orders may
        // contradict each other, so this is frequently infeasible -- which is
        // exactly what we want when starting from infeasible solutions.
        static MachineChain random_infeasible(const Instance &inst, std::mt19937 &rng);

        // Wraps an existing per-machine order.
        MachineChain(const Instance &inst, std::vector<std::vector<int>> order);

        // --- Access ------------------------------------------------------------
        const std::vector<std::vector<int>> &order() const { return order_; }
        int num_machines() const { return static_cast<int>(order_.size()); }

        // Swaps two adjacent operations in machine k (positions p and p+1).
        // This is the neighborhood move: it may make a feasible chain infeasible
        // or vice versa.
        void swap_adjacent(int machine, int pos);

        // Swaps operations at positions a and b within machine k.
        void swap_in_machine(int machine, int a, int b);

        // --- Evaluation --------------------------------------------------------
        // Builds the disjunctive graph and runs Kahn's algorithm. Returns the
        // makespan, the number of operations stuck in cycles, and feasibility.
        ChainEvaluation evaluate() const;

        // --- Output ------------------------------------------------------------
        // Machine-chain format, like sol1.txt: one line per machine with
        // [job, op, dur] triples.
        std::string to_string() const;

    private:
        const Instance *inst_;
        std::vector<std::vector<int>> order_; // order_[machine] = op ids in order

        // Helpers to translate between op_id and (job, pos).
        int op_id(int job, int pos) const { return job * inst_->num_machines() + pos; }
        int job_of(int op_id) const { return op_id / inst_->num_machines(); }
        int pos_of(int op_id) const { return op_id % inst_->num_machines(); }
    };

} // namespace jssp

#endif // JSSP_MACHINE_CHAIN_HPP
