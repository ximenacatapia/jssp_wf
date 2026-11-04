#include "chain_cost.hpp"

namespace jssp
{
    double chain_cost(const Instance &inst, const MachineChain &chain)
    {
        ChainEvaluation ev = chain.evaluate();

        const long long lo = inst.lower_bound();
        const long long hi = inst.upper_bound();
        const long long range = hi - lo;

        // Normalized makespan term
        double normalized = 0.0;
        if (range > 0)
            normalized = static_cast<double>(ev.makespan - lo) /
                         static_cast<double>(range);

        // Penalty: cotaInferior * (operations in cycles).zero when feasible
        double penalty =
            static_cast<double>(lo) * static_cast<double>(ev.nodes_in_cycle);

        return normalized + penalty;
    }
} // namespace jssp
