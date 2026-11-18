// Solver/backtracking/preplaced.cpp
#include "nqueens.h"
#include <vector>
#include <utility>
#include <cmath>

namespace {
void dfs(int r, int n, const std::vector<int>& fixed, int pCols, int cols, int d1, int d2, std::vector<int>& cur, std::vector<std::vector<int>>& sols) {
    if (r == n) {
        sols.push_back(cur);
        NQ_SNAPSHOT(nq::queensOf(cur), "solusi " + std::to_string(sols.size()));
        return;
    }
    if (fixed[r] >= 0) {
        int bit = 1 << fixed[r];
        if ((cols | d1 | d2) & bit) return;
        cur[r] = fixed[r];
        dfs(r + 1, n, fixed, pCols, cols | bit, (d1 | bit) << 1, (d2 | bit) >> 1, cur, sols);
    } else {
        int safe = (~(cols | d1 | d2 | pCols)) & ((1 << n) - 1);
        while (safe) {
            int bit = safe & -safe;
            cur[r] = __builtin_ctz(bit);
            dfs(r + 1, n, fixed, pCols, cols | bit, (d1 | bit) << 1, (d2 | bit) >> 1, cur, sols);
            safe &= safe - 1;
        }
    }
}
}

std::vector<std::vector<int>> solvePreplaced(int n, const std::vector<std::pair<int, int>>& pre) {
    std::vector<std::vector<int>> sols;
    std::vector<int> fixed(n, -1);
    int pCols = 0;
    for (auto& p : pre) {
        if (fixed[p.first] != -1 || (pCols & (1 << p.second))) return sols;
        for (auto& q : pre) {
            if (&p != &q && std::abs(p.first - q.first) == std::abs(p.second - q.second)) return sols;
        }
        fixed[p.first] = p.second;
        pCols |= (1 << p.second);
    }
    std::vector<int> cur(n);
    dfs(0, n, fixed, pCols, 0, 0, 0, cur, sols);
    return sols;
}
