// Solver/analgore/normal.cpp
#include "nqueens.h"
#include <vector>
#include <string>

namespace {
void dfs(int r, int n, int cols, int d1, int d2, std::vector<int>& cur, std::vector<std::vector<int>>& sols) {
    if (r == n) {
        sols.push_back(cur);
        NQ_SNAPSHOT(nq::queensOf(cur), "solusi " + std::to_string(sols.size()));
        return;
    }
    int safe = (~(cols | d1 | d2)) & ((1 << n) - 1);
    if (!safe) NQ_SNAPSHOT(nq::queensOf(cur), "dead end row " + std::to_string(r));
    while (safe) {
        int bit = safe & -safe;
        cur[r] = __builtin_ctz(bit);
        NQ_SNAPSHOT(nq::queensOf(cur), "place row " + std::to_string(r));
        dfs(r + 1, n, cols | bit, (d1 | bit) << 1, (d2 | bit) >> 1, cur, sols);
        cur[r] = -1;
        NQ_SNAPSHOT(nq::queensOf(cur), "backtrack row " + std::to_string(r));
        safe &= safe - 1;
    }
}
}

std::vector<std::vector<int>> solveNQueens(int n) {
    std::vector<std::vector<int>> sols;
    // Bound n before allocating or shifting. The browser supports at most 12.
    if (n < 1 || n > 12) return sols;
    std::vector<int> cur(n, -1);
    dfs(0, n, 0, 0, 0, cur, sols);
    return sols;
}
