#ifndef JSSP_SCHEDULE_HPP
#define JSSP_SCHEDULE_HPP

#include <vector>

#include "instance.hpp"
#include "solution.hpp"

/*  Decoder: turn an operation-based permutation into an actual schedule. (permutation ->, schedule -> makespan).*/
namespace jssp
{

    // Start/finish time of one scheduled operation, kept for inspection and for
    // printing a schedule. Indexed the same way as the instance's operations.
    struct ScheduledOp
    {
        int job;
        int position; // operation index within the job
        int machine;
        int start;
        int end;
    };

    class Schedule
    {
    public:
        // Decodes a solution into a full schedule for the given instance
        Schedule(const Instance &inst, const Solution &sol);

        // The objective value, is the time at which all operations have finished
        int makespan() const { return makespan_; }

        // All scheduled operations, in the order they were placed
        const std::vector<ScheduledOp> &operations() const { return ops_; }

    private:
        int makespan_ = 0;
        std::vector<ScheduledOp> ops_;
    };

    // decode and return only the makespan.
    int makespan_of(const Instance &inst, const Solution &sol);

} // namespace jssp

#endif // JSSP_SCHEDULE_HPP
