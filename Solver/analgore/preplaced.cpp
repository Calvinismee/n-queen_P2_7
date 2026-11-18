// Solver/analgore/preplaced.cpp
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
        if ((cols | d1 | d2) & bit) {
            NQ_SNAPSHOT(nq::queensOf(cur), "locked queen conflict row " + std::to_string(r));
            return;
        }
        cur[r] = fixed[r];
        NQ_SNAPSHOT(nq::queensOf(cur), "locked row " + std::to_string(r));
        dfs(r + 1, n, fixed, pCols, cols | bit, (d1 | bit) << 1, (d2 | bit) >> 1, cur, sols);
        // Locked queens stay visible when the search backtracks.
    } else {
        int safe = (~(cols | d1 | d2 | pCols)) & ((1 << n) - 1);
        if (!safe) NQ_SNAPSHOT(nq::queensOf(cur), "dead end row " + std::to_string(r));
        while (safe) {
            int bit = safe & -safe;
            cur[r] = __builtin_ctz(bit);
            NQ_SNAPSHOT(nq::queensOf(cur), "place row " + std::to_string(r));
            dfs(r + 1, n, fixed, pCols, cols | bit, (d1 | bit) << 1, (d2 | bit) >> 1, cur, sols);
            cur[r] = -1;
            NQ_SNAPSHOT(nq::queensOf(cur), "backtrack row " + std::to_string(r));
            safe &= safe - 1;
        }
    }
}
}

std::vector<std::vector<int>> solvePreplaced(int n, const std::vector<std::pair<int, int>>& pre) {
    std::vector<std::vector<int>> sols;
    if (n < 1 || n > 12) return sols;
    // Validate every coordinate before indexing, shifting or subtracting coordinates.
    for (const auto& p : pre)
        if (p.first < 0 || p.first >= n || p.second < 0 || p.second >= n) return sols;
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
    std::vector<int> cur = fixed;
    dfs(0, n, fixed, pCols, 0, 0, 0, cur, sols);
    return sols;
}
