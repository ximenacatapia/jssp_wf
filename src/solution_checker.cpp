#include "solution_checker.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>

namespace jssp
{

    namespace
    {

        std::vector<std::vector<std::pair<int, int>>> parse_machine_chains(
            const std::string &path, bool &ok, std::string &err)
        {
            ok = false;
            std::ifstream in(path);
            if (!in)
            {
                err = "Could not open solution file: " + path;
                return {};
            }

            std::vector<std::vector<std::pair<int, int>>> machines;
            std::string line;
            while (std::getline(in, line))
            {
                // Skip footer lines
                if (line.find(':') != std::string::npos)
                    continue;
                // Skip blank lines.
                if (line.find_first_not_of(" \t\r\n") == std::string::npos)
                    continue;

                // Replace every non-digit with a space
                std::string cleaned = line;
                for (char &c : cleaned)
                {
                    if (!(c >= '0' && c <= '9') && c != '-')
                        c = ' ';
                }
                std::istringstream ss(cleaned);
                std::vector<int> nums;
                int x;
                while (ss >> x)
                    nums.push_back(x);

                if (nums.empty())
                    continue;

                if (nums.size() % 3 != 0)
                {
                    err = "A machine line does not have a multiple of 3 integers.";
                    return {};
                }

                std::vector<std::pair<int, int>> ops;
                for (size_t i = 0; i < nums.size(); i += 3)
                {
                    ops.push_back({nums[i], nums[i + 1]});
                }
                machines.push_back(std::move(ops));
            }

            ok = true;
            return machines;
        }

    } // namespace

    CheckResult check_solution_file(const Instance &inst, const std::string &path)
    {
        CheckResult r;

        bool ok;
        std::string err;
        auto chains = parse_machine_chains(path, ok, err);
        if (!ok)
        {
            r.message = err;
            return r;
        }

        const int n = inst.num_jobs();
        const int m = inst.num_machines();

        if (static_cast<int>(chains.size()) != m)
        {
            r.message = "File has " + std::to_string(chains.size()) +
                        " machine lines, instance has " + std::to_string(m) +
                        " machines.";
            return r;
        }

        // Count operations read.
        int total = 0;
        for (const auto &c : chains)
            total += static_cast<int>(c.size());
        r.operations = total;
        if (total != inst.num_operations())
        {
            r.message = "File has " + std::to_string(total) +
                        " operations, instance has " +
                        std::to_string(inst.num_operations()) + ".";
            return r;
        }
        r.parsed = true;

        // Simulate the schedule
        std::vector<size_t> head(m, 0);        // next position in each machine chain
        std::vector<int> next_op_of_job(n, 0); // next op index expected per job
        std::vector<long long> job_ready(n, 0);
        std::vector<long long> machine_ready(m, 0);

        int scheduled = 0;
        long long makespan = 0;

        bool progress = true;
        while (scheduled < total && progress)
        {
            progress = false;

            for (int k = 0; k < m; ++k)
            {
                if (head[k] >= chains[k].size())
                    continue;

                const int job = chains[k][head[k]].first;
                const int op = chains[k][head[k]].second;

                if (next_op_of_job[job] != op)
                    continue;

                long long start = std::max(job_ready[job], machine_ready[k]);
                long long finish = start + inst.duration(job, op);

                job_ready[job] = finish;
                machine_ready[k] = finish;
                next_op_of_job[job] = op + 1;
                ++head[k];
                ++scheduled;
                makespan = std::max(makespan, finish);
                progress = true;
            }
        }

        if (scheduled < total)
        {
            r.feasible = false;
            r.message = "Deadlock: " + std::to_string(total - scheduled) +
                        " operations could not be scheduled (the solution has a "
                        "cycle / is infeasible).";
            return r;
        }

        r.feasible = true;
        r.makespan = static_cast<int>(makespan);
        r.message = "OK";
        return r;
    }

} // namespace jssp
