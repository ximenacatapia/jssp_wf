#ifndef JSSP_SOLUTION_CHECKER_HPP
#define JSSP_SOLUTION_CHECKER_HPP

#include <string>

#include "instance.hpp"

//  Solution checker: load a solution file (machine-chain format) and evaluate
//  it against an instance with THIS program's decoder.
namespace jssp
{

    struct CheckResult
    {
        bool parsed = false; // the file was read and matched the instance
        bool feasible = false;
        int makespan = 0;   // makespan if feasible
        int operations = 0; // operations read
        std::string message;
    };

    // Loads the solution at 'path' and evaluates it against 'inst'.
    CheckResult check_solution_file(const Instance &inst, const std::string &path);

} // namespace jssp

#endif // JSSP_SOLUTION_CHECKER_HPP
