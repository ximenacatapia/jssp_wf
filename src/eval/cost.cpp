#include "cost.hpp"

#include "schedule.hpp"

namespace jssp
{

    double normalize_makespan(const Instance &inst, int makespan)
    {
        const long long lo = inst.lower_bound();
        const long long hi = inst.upper_bound();

        // Degenerate instance
        const long long range = hi - lo;
        if (range <= 0)
        {
            return 0.0;
        }

        return static_cast<double>(makespan - lo) / static_cast<double>(range);
    }

    double cost(const Instance &inst, const Solution &sol)
    {
        const int mk = makespan_of(inst, sol);
        return normalize_makespan(inst, mk);
    }

} // namespace jssp