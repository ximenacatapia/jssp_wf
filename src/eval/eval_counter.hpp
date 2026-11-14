#ifndef JSSP_EVAL_COUNTER_HPP
#define JSSP_EVAL_COUNTER_HPP

/*
Global evaluation counter.
*/

namespace jssp
{
    // thread_local so each parallel seed run counts its own evaluations.
    inline thread_local long long g_eval_count = 0;

    inline void reset_eval_count() { g_eval_count = 0; }
    inline long long eval_count() { return g_eval_count; }
    inline void bump_eval_count() { ++g_eval_count; }

} // namespace jssp

#endif // JSSP_EVAL_COUNTER_HPP
