#include "chain_cost.hpp"

namespace jssp
{
    double chain_cost(const Instance &inst, const MachineChain &chain)
    {
        const ChainEvaluation &ev = chain.evaluate();

        const long long lo = inst.lower_bound();
        const long long hi = inst.upper_bound();
        const long long range = hi - lo;

        // Normalized makespan term (0 if the bounds coincide; degenerate case).
        double normalized = 0.0;
        if (range > 0)
            normalized = static_cast<double>(ev.makespan - lo) /
                         static_cast<double>(range);

        // Penalty term: cotaInferior * (operations in cycles). Zero when
        // feasible, so a feasible chain's cost equals the normalized makespan.
        double penalty =
            static_cast<double>(lo) * static_cast<double>(ev.nodes_in_cycle);

        return normalized + penalty;
    }
} // namespace jssp