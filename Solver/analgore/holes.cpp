// Solver/backtracking/holes.cpp
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
    while (safe) {
        int bit = safe & -safe;
        cur[r] = __builtin_ctz(bit);
        dfs(r + 1, n, fMask, cols | bit, (d1 | bit) << 1, (d2 | bit) >> 1, cur, sols);
        safe &= safe - 1;
    }
}
}

std::vector<std::vector<int>> solveHoles(int n, const std::vector<std::string>& grid) {
    std::vector<std::vector<int>> sols;
    std::vector<int> fMask(n, 0);
    for (int r = 0; r < n; ++r)
        for (int c = 0; c < n; ++c)
            if (grid[r][c] != 'H') fMask[r] |= (1 << c);
    std::vector<int> cur(n);
    dfs(0, n, fMask, 0, 0, 0, cur, sols);
    return sols;
}
