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
        int makespan = 0; // longest path if feasible, partial otherwise
        int nodes_in_cycle = 0;
        bool feasible = false;
        std::vector<int> cycle_ops;
    };

    class MachineChain
    {
    public:
        static MachineChain random_feasible(const Instance &inst, std::mt19937 &rng);

        static MachineChain random_nonfeasible(const Instance &inst, std::mt19937 &rng);

        MachineChain(const Instance &inst, std::vector<std::vector<int>> order);

        // Access
        const std::vector<std::vector<int>> &order() const { return order_; }
        int num_machines() const { return static_cast<int>(order_.size()); }

        // Swaps two adjacent operations in machine k
        void swap_adjacent(int machine, int pos);

        // Swaps operations at positions a and b within machine k.
        void swap_in_machine(int machine, int a, int b);

        int machine_of(int op_id) const;
        int position_in_machine(int op_id) const;

        // Evaluation
        const ChainEvaluation &evaluate() const;

        // Output
        std::string to_string() const;

    private:
        const Instance *inst_;
        std::vector<std::vector<int>> order_; // order_[machine] = op ids in order

        mutable bool valid_ = false;
        mutable ChainEvaluation cache_;
        void invalidate() { valid_ = false; }

        int op_id(int job, int pos) const { return job * inst_->num_machines() + pos; }
        int job_of(int op_id) const { return op_id / inst_->num_machines(); }
        int pos_of(int op_id) const { return op_id % inst_->num_machines(); }
    };

} // namespace jssp

#endif // JSSP_MACHINE_CHAIN_HPP
