#ifndef JSSP_COST_HPP
#define JSSP_COST_HPP

#include "instance.hpp"
#include "solution.hpp"

//  Cost function: the normalized objective from the JSSP class paper.
//      f(X) = (makespan(X) - lowerBound) / (upperBound - lowerBound)

namespace jssp
{

    // Normalized cost of a solution (the lower is better). Returns a value in [0, 1] for
    // any schedule whose makespan lies between the two bounds.
    double cost(const Instance &inst, const Solution &sol);

    // Same but for a raw makespan value.
    double normalize_makespan(const Instance &inst, int makespan);

} // namespace jssp

#endif // JSSP_COST_HPP
