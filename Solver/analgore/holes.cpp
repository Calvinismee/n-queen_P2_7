// Solver/analgore/holes.cpp
#include "nqueens.h"
#include <vector>
#include <string>

namespace {
void dfs(int r, int n, const std::vector<int>& fMask, int cols, int d1, int d2, std::vector<int>& cur, std::vector<std::vector<int>>& sols) {
    if (r == n) {
        sols.push_back(cur);
        NQ_SNAPSHOT(nq::queensOf(cur), "solusi " + std::to_string(sols.size()));
        return;
    }
    int safe = (~(cols | d1 | d2)) & fMask[r] & ((1 << n) - 1);
    if (!safe) NQ_SNAPSHOT(nq::queensOf(cur), "dead end row " + std::to_string(r));
    while (safe) {
        int bit = safe & -safe;
        cur[r] = __builtin_ctz(bit);
        NQ_SNAPSHOT(nq::queensOf(cur), "place row " + std::to_string(r));
        dfs(r + 1, n, fMask, cols | bit, (d1 | bit) << 1, (d2 | bit) >> 1, cur, sols);
        cur[r] = -1;
        NQ_SNAPSHOT(nq::queensOf(cur), "backtrack row " + std::to_string(r));
        safe &= safe - 1;
    }
}
}

std::vector<std::vector<int>> solveHoles(int n, const std::vector<std::string>& grid) {
    std::vector<std::vector<int>> sols;
    if (n < 1 || n > 12 || grid.size() != static_cast<size_t>(n)) return sols;
    for (const auto& row : grid)
        if (row.size() != static_cast<size_t>(n) || row.find_first_not_of(".H") != std::string::npos) return sols;
    std::vector<int> fMask(n, 0);
    for (int r = 0; r < n; ++r)
        for (int c = 0; c < n; ++c)
            if (grid[r][c] != 'H') fMask[r] |= (1 << c);
    std::vector<int> cur(n, -1);
    dfs(0, n, fMask, 0, 0, 0, cur, sols);
    return sols;
}
