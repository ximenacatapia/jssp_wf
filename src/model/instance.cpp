#include "instance.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace jssp
{

    Instance Instance::from_file(const std::string &path)
    {
        std::ifstream in(path);
        if (!in)
        {
            throw std::runtime_error("Could not open instance: " + path);
        }

        Instance inst;

        // fist line, read line by line to skip blank lines and comments.
        std::string line;
        auto next_data_line = [&](std::string &out) -> bool
        {
            while (std::getline(in, out))
            {
                size_t start = out.find_first_not_of(" \t\r\n");
                if (start == std::string::npos)
                    continue; // blank line
                if (out[start] == '#')
                    continue; // comment
                out = out.substr(start);
                return true;
            }
            return false;
        };

        if (!next_data_line(line))
        {
            throw std::runtime_error("Empty instance: " + path);
        }
        {
            std::istringstream header(line);
            if (!(header >> inst.n_ >> inst.m_ >> inst.cu_file_ >> inst.ci_file_))
            {
                throw std::runtime_error(
                    "Invalid header (expected: n m cu ci) in " + path);
            }
        }

        if (inst.n_ <= 0 || inst.m_ <= 0)
        {
            throw std::runtime_error("n and m must be positive in " + path);
        }

        inst.jobs_.assign(inst.n_, {});

        // next n lines
        for (int i = 0; i < inst.n_; ++i)
        {
            if (!next_data_line(line))
            {
                throw std::runtime_error(
                    "Missing jobs: expected " + std::to_string(inst.n_) +
                    " in " + path);
            }
            std::istringstream row(line);
            inst.jobs_[i].reserve(inst.m_);
            for (int pos = 0; pos < inst.m_; ++pos)
            {
                int mach, dur;
                if (!(row >> mach >> dur))
                {
                    throw std::runtime_error(
                        "Job " + std::to_string(i) + ": expected " +
                        std::to_string(inst.m_) +
                        " (machine duration) pairs in " + path);
                }
                if (mach < 0 || mach >= inst.m_)
                {
                    throw std::runtime_error(
                        "Machine out of range in job " + std::to_string(i) +
                        " (value " + std::to_string(mach) + ") in " + path);
                }
                if (dur < 0)
                {
                    throw std::runtime_error(
                        "Negative duration in job " + std::to_string(i) +
                        " in " + path);
                }
                Operation op{i, pos, mach, dur};
                inst.jobs_[i].push_back(op);
                inst.ops_.push_back(op);
            }
        }

        inst.compute_bounds();
        return inst;
    }

    Instance Instance::from_data(
        int num_machines,
        const std::vector<std::vector<std::pair<int, int>>> &jobs)
    {
        Instance inst;
        inst.n_ = static_cast<int>(jobs.size());
        inst.m_ = num_machines;

        if (inst.n_ <= 0 || inst.m_ <= 0)
        {
            throw std::runtime_error("n and m must be positive");
        }

        inst.jobs_.assign(inst.n_, {});
        for (int i = 0; i < inst.n_; ++i)
        {
            int pos = 0;
            for (const auto &[mach, dur] : jobs[i])
            {
                if (mach < 0 || mach >= inst.m_)
                {
                    throw std::runtime_error("Machine out of range in job " +
                                             std::to_string(i));
                }
                if (dur < 0)
                {
                    throw std::runtime_error("Negative duration in job " +
                                             std::to_string(i));
                }
                Operation op{i, pos, mach, dur};
                inst.jobs_[i].push_back(op);
                inst.ops_.push_back(op);
                ++pos;
            }
        }

        // The header bounds are unknown for in-memory data; leave them at 0.
        inst.cu_file_ = 0;
        inst.ci_file_ = 0;
        inst.compute_bounds();
        return inst;
    }

    void Instance::compute_bounds()
    {
        // upper bound
        std::vector<long long> job_time(n_, 0);
        std::vector<long long> machine_time(m_, 0);

        for (int pos = 0; pos < m_; ++pos)
        { // for each operation layer
            for (int j = 0; j < n_; ++j)
            { // for each job
                const Operation &op = jobs_[j][pos];
                long long start =
                    std::max(job_time[j], machine_time[op.machine]);
                long long finish = start + op.duration;
                job_time[j] = finish;
                machine_time[op.machine] = finish;
            }
        }

        // Upper bound: the sum of ALL durations (process every operation
        // serially, one by one)
        upper_bound_ = 0;
        for (const auto &op : ops_)
        {
            upper_bound_ += op.duration;
        }

        // maxJobs: the job with the largest total duration.
        long long max_jobs = 0;
        for (int i = 0; i < n_; ++i)
        {
            long long s = 0;
            for (const auto &op : jobs_[i])
                s += op.duration;
            max_jobs = std::max(max_jobs, s);
        }

        // maxMachines: the machine with the largest total load.
        std::vector<long long> load(m_, 0);
        for (const auto &op : ops_)
        {
            load[op.machine] += op.duration;
        }
        long long max_machines = 0;
        for (int k = 0; k < m_; ++k)
        {
            max_machines = std::max(max_machines, load[k]);
        }

        lower_bound_ = std::max(max_jobs, max_machines);
    }

} // namespace jssp
