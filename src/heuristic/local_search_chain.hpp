#ifndef JSSP_LOCAL_SEARCH_CHAIN_HPP
#define JSSP_LOCAL_SEARCH_CHAIN_HPP

#include <random>

#include "../eval/eval_counter.hpp"
#include "../model/instance.hpp"
#include "../model/machine_chain.hpp"
#include "../model/solution.hpp"
#include "local_search.hpp"
#include "local_search_n5.hpp"
#include "repr_chain.hpp"
#include "repr_permutation.hpp"

/*
Local search for the machine-chain representation, in two phases:
Basically when we start from non-feasible we do the logic we already has until we get a feasible and
from there we use the N5 that we used in permutation creating a sort of bridge to get better results
*/

namespace jssp
{

#ifndef JSSP_DISABLE_N5_OVERLOAD
    template <>
    inline MachineChain local_search<MachineChain>(const Instance &inst,
                                                   const MachineChain &start,
                                                   const LocalSearchParams &params,
                                                   std::mt19937 &rng)
    {
        // p1: repair / descend with the sampled cycle-directed search.
        MachineChain repaired = sampled_local_search(inst, start, params, rng);

        // If still non-feasible, there is no critical path to work with: return the
        // best repair attempt (its penalized cost still guides the Water-Flow).
        std::vector<int> seq = repaired.to_sequence();
        if (seq.empty())
            return repaired;

        // Phase 2: improve the feasible schedule with N5 on the permutation.
        Solution perm(inst, seq);
        Solution improved = n5_tabu_search(inst, perm, params, rng);

        MachineChain bridged = MachineChain::from_sequence(inst, improved.sequence());

        // Keep whichever is better
        return (repr_cost(inst, bridged) < repr_cost(inst, repaired)) ? bridged
                                                                      : repaired;
    }
#endif // JSSP_DISABLE_N5_OVERLOAD

} // namespace jssp

#endif // JSSP_LOCAL_SEARCH_CHAIN_HPP
