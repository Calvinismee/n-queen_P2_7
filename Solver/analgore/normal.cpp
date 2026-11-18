// Solver/backtracking/normal.cpp
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
    while (safe) {
        int bit = safe & -safe;
        cur[r] = __builtin_ctz(bit);
        dfs(r + 1, n, cols | bit, (d1 | bit) << 1, (d2 | bit) >> 1, cur, sols);
        safe &= safe - 1;
    }
}
}

std::vector<std::vector<int>> solveNQueens(int n) {
    std::vector<std::vector<int>> sols;
    if (n <= 0) return sols;
    std::vector<int> cur(n);
    dfs(0, n, 0, 0, 0, cur, sols);
    return sols;
}
