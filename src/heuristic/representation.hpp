#ifndef JSSP_REPRESENTATION_HPP
#define JSSP_REPRESENTATION_HPP

#include <random>

#include "../model/instance.hpp"

//  Representation traits for the generic Water-Flow, so we can start by non-feasible or feasible

namespace jssp
{
    // Marker for how a run should start
    enum class StartMode
    {
        FEASIBLE,
        NONFEASIBLE,
    };

    template <class Repr>
    Repr repr_random(const Instance &inst, std::mt19937 &rng, StartMode mode);
} // namespace jssp

#endif // JSSP_REPRESENTATION_HPP
