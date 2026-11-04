#ifndef JSSP_CHAIN_COST_HPP
#define JSSP_CHAIN_COST_HPP

#include "../model/instance.hpp"
#include "../model/machine_chain.hpp"

//  Cost function for the machine-chain representation (with non-feasibility
//  penalty), just as in the JSSP pdf:
//    f(X) = (makespan - cotaInferior) / (cotaSuperior - cotaInferior)
//           + cotaInferior * (operations in cycles)
namespace jssp
{
    double chain_cost(const Instance &inst, const MachineChain &chain);
}

#endif // JSSP_CHAIN_COST_HPP
