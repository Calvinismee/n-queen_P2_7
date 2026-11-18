// Solver/backtracking/walled.cpp
#include "nqueens.h"
#include <vector>
#include <string>
#include <algorithm>

namespace {
struct Search {
    int n;
    const std::vector<std::string>& grid;
    std::vector<int> rowMax;
    std::vector<std::pair<int, int>> cur;
    std::vector<std::vector<std::pair<int, int>>> best;
    size_t bestN = 0;

    Search(int n_, const std::vector<std::string>& g) : n(n_), grid(g) {
        rowMax.assign(n, 0);
        for (int r = 0; r < n; ++r) {
            int segs = 0, c = 0;
            while (c < n) {
                if (grid[r][c] == 'W') { c++; continue; }
                segs++;
                while (c < n && grid[r][c] != 'W') c++;
            }
            rowMax[r] = segs;
        }
    }

    bool hits(int r1, int c1, int r2, int c2) {
        if (r1 != r2 && c1 != c2 && std::abs(r1 - r2) != std::abs(c1 - c2)) return false;
        int dr = (r2 > r1) - (r2 < r1), dc = (c2 > c1) - (c2 < c1);
        for (int r = r1 + dr, c = c1 + dc; r != r2 || c != c2; r += dr, c += dc)
            if (grid[r][c] == 'W') return false;
        return true;
    }

    void gen(int r, int c, std::vector<int>& picked, std::vector<std::vector<int>>& opts) {
        if (c >= n) { opts.push_back(picked); return; }
        if (grid[r][c] == 'W') { gen(r, c + 1, picked, opts); return; }
        int start = c;
        while (c < n && grid[r][c] != 'W') c++;
        gen(r, c, picked, opts);
        for (int i = start; i < c; ++i) {
            picked.push_back(i);
            gen(r, c, picked, opts);
            picked.pop_back();
        }
    }

    void dfs(int r, int count) {
        int bound = 0;
        for (int i = r; i < n; ++i) bound += rowMax[i];
        if (count + bound < (int)bestN) return;

        if (r == n) {
            if (cur.size() > bestN) { bestN = cur.size(); best = {cur}; NQ_SNAPSHOT(cur, "rekord: " + std::to_string(bestN)); }
            else if (cur.size() == bestN && bestN > 0) best.push_back(cur);
            return;
        }

        std::vector<std::vector<int>> opts;
        std::vector<int> picked;
        gen(r, 0, picked, opts);
        std::sort(opts.begin(), opts.end(), [](const auto& a, const auto& b) { return a.size() > b.size(); });

        for (auto& cols : opts) {
            bool ok = true;
            for (int c : cols) for (auto& q : cur) if (hits(q.first, q.second, r, c)) { ok = false; break; }
            if (!ok) continue;
            for (int c : cols) cur.push_back({r, c});
            dfs(r + 1, count + cols.size());
            for (size_t i = 0; i < cols.size(); ++i) cur.pop_back();
        }
    }
};
}

std::vector<std::vector<std::pair<int, int>>> solveWalled(int n, const std::vector<std::string>& grid) {
    Search s(n, grid);
    s.dfs(0, 0);
    return s.best.empty() ? std::vector<std::vector<std::pair<int, int>>>() : s.best;
}
