#ifndef JSSP_SOLUTION_WRITER_HPP
#define JSSP_SOLUTION_WRITER_HPP

#include <string>

#include "../model/instance.hpp"
#include "../model/solution.hpp"

namespace jssp
{

    void write_solution(const Instance &inst,
                        const Solution &sol,
                        const std::string &path);

} // namespace jssp

#endif // JSSP_SOLUTION_WRITER_HPP
