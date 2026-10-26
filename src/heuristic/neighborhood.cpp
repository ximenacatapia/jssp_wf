#include "neighborhood.hpp"

namespace jssp
{

    SwapMove random_move(const Solution &sol, std::mt19937 &rng)
    {
        const int len = sol.size();
        std::uniform_int_distribution<int> pos(0, len - 1);

        // Pick two positions with different job ids. For any instance with at least
        // two jobs there are always such pairs, so this loop terminates quickly.
        int i = 0, j = 0;
        do
        {
            i = pos(rng);
            j = pos(rng);
        } while (sol[i] == sol[j]);

        return SwapMove{i, j};
    }

    Solution apply_move(const Solution &sol, SwapMove move)
    {
        Solution neighbor = sol; // copy
        neighbor.swap_positions(move.i, move.j);
        return neighbor;
    }

    std::vector<SwapMove> all_moves(const Solution &sol)
    {
        const int len = sol.size();
        std::vector<SwapMove> moves;

        // Only pairs (i < j) whose ids differ change the sequence. Pairs with equal
        // ids are null moves and are skipped.
        for (int i = 0; i < len; ++i)
        {
            for (int j = i + 1; j < len; ++j)
            {
                if (sol[i] != sol[j])
                {
                    moves.push_back(SwapMove{i, j});
                }
            }
        }
        return moves;
    }

} // namespace jssp
