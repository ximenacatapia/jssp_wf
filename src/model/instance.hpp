#ifndef JSSP_INSTANCE_HPP
#define JSSP_INSTANCE_HPP

#include <string>
#include <vector>

namespace jssp
{
    // An operation O_ij: the j-th operation of job i.
    struct Operation
    {
        int job;      // job this operation belongs to (0..n-1)
        int position; // position within the job
        int machine;  // machine that processes it (0..m-1)
        int duration; // processing time (p_ij)
    };

    class Instance
    {
    public:
        // Loads an instance from a file
        static Instance from_file(const std::string &path);

        // Builds an instance directly from in-memory data.
        static Instance from_data(
            int num_machines,
            const std::vector<std::vector<std::pair<int, int>>> &jobs);

        int num_jobs() const { return n_; }
        int num_machines() const { return m_; }

        // Total number of operations = n * m (each job has m operations).
        int num_operations() const { return static_cast<int>(ops_.size()); }

        long long upper_bound_file() const { return cu_file_; }
        long long lower_bound_file() const { return ci_file_; }

        const std::vector<Operation> &job(int i) const { return jobs_[i]; }

        int duration(int job, int pos) const { return jobs_[job][pos].duration; }

        int machine(int job, int pos) const { return jobs_[job][pos].machine; }

        long long upper_bound() const { return upper_bound_; }

        long long lower_bound() const { return lower_bound_; }

    private:
        void compute_bounds();

        int n_ = 0; // jobs
        int m_ = 0; // machines
        long long cu_file_ = 0;
        long long ci_file_ = 0;

        // jobs_[i] = ordered list of operations of job i
        std::vector<std::vector<Operation>> jobs_;

        // ops_ = all operations in a list
        std::vector<Operation> ops_;

        long long upper_bound_ = 0;
        long long lower_bound_ = 0;
    };

} // namespace jssp

#endif // JSSP_INSTANCE_HPP
