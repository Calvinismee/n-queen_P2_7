#include "nqueens.h"

#include <cmath>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

namespace {

// =================================================================================================
// KASUS 1: Classic / Normal N-Queens
// Algoritma: Backtracking (Runut Balik) dengan Bitwise Constraint Pruning
// =================================================================================================

/**
 * Fungsi rekursif DFS untuk menempatkan ratu pada setiap baris r (0 s.d. n-1).
 * 
 * Parameter pembatas (constraints):
 * - colsMask  : Bitmask kolom yang sudah terancam oleh ratu di baris sebelumnya.
 * - diag1Mask : Bitmask diagonal serong kiri (r - c) yang terancam, digeser ke kiri tiap baris.
 * - diag2Mask : Bitmask diagonal serong kanan (r + c) yang terancam, digeser ke kanan tiap baris.
 */
void dfsNormal(int r, int n, int colsMask, int diag1Mask, int diag2Mask,
               std::vector<int>& current, std::vector<std::vector<int>>& solutions) {
    // Basis rekurensi: Jika semua baris 0..(n-1) berhasil diisi ratu secara aman
    if (r == n) {
        solutions.push_back(current);
        NQ_SNAPSHOT(nq::queensOf(current), "solusi " + std::to_string(solutions.size()));
        return;
    }

    // Hitung posisi kolom yang aman (tersedia) pada baris r
    int available = (~(colsMask | diag1Mask | diag2Mask)) & ((1 << n) - 1);

    // Eksplorasi setiap kolom yang valid (pruning: kolom terancam otomatis tidak dikunjungi)
    while (available) {
        // Ambil bit aman paling kanan (LSB)
        int bit = available & -available;
        int c = __builtin_ctz(bit); // Mendapatkan indeks kolom dari bitmask

        current[r] = c;

        // Rekursi ke baris r + 1 dengan memperbarui ancaman kolom & diagonal
        dfsNormal(r + 1, n, colsMask | bit, (diag1Mask | bit) << 1, (diag2Mask | bit) >> 1, current, solutions);

        // Hapus bit yang sudah dieksplorasi (backtrack ke pilihan kolom berikutnya)
        available &= available - 1;
    }
}

// =================================================================================================
// KASUS 2: Pre-placed N-Queens
// Algoritma: Backtracking dengan Pra-kondisi Ratu Terkunci (Fixed Queens)
// =================================================================================================

/**
 * Fungsi rekursif DFS untuk kasus di mana ada P ratu yang posisinya sudah terkunci.
 */
void dfsPreplaced(int r, int n, const std::vector<int>& fixedCol, int preplacedColsMask,
                  int colsMask, int diag1Mask, int diag2Mask,
                  std::vector<int>& current, std::vector<std::vector<int>>& solutions) {
    if (r == n) {
        solutions.push_back(current);
        NQ_SNAPSHOT(nq::queensOf(current), "solusi " + std::to_string(solutions.size()));
        return;
    }

    // Cabang A: Jika baris r adalah baris dengan ratu terkunci (pre-placed)
    if (fixedCol[r] >= 0) {
        int c = fixedCol[r];
        int bit = 1 << c;

        // Cek kendala: Jika ratu terkunci ini ternyata terancam ratu di atasnya, pangkas cabang (prune)
        if ((colsMask | diag1Mask | diag2Mask) & bit) {
            return;
        }

        current[r] = c;
        dfsPreplaced(r + 1, n, fixedCol, preplacedColsMask,
                     colsMask | bit, (diag1Mask | bit) << 1, (diag2Mask | bit) >> 1,
                     current, solutions);
    } 
    // Cabang B: Jika baris r bebas (tidak ada ratu terkunci)
    else {
        // Kolom yang boleh dipilih adalah kolom yang tidak terancam DAN bukan milik ratu terkunci lain
        int available = (~(colsMask | diag1Mask | diag2Mask | preplacedColsMask)) & ((1 << n) - 1);

        while (available) {
            int bit = available & -available;
            int c = __builtin_ctz(bit);

            current[r] = c;
            dfsPreplaced(r + 1, n, fixedCol, preplacedColsMask,
                         colsMask | bit, (diag1Mask | bit) << 1, (diag2Mask | bit) >> 1,
                         current, solutions);

            available &= available - 1;
        }
    }
}

} // namespace

// =================================================================================================
// IMPLEMENTASI FUNGSI RESMI (Hanya Kasus 1 & 2)
// =================================================================================================

// Kasus 1: Classic N-Queens
std::vector<std::vector<int>> solveNQueens(int n) {
    std::vector<std::vector<int>> solutions;
    if (n <= 0) return solutions;

    // Tabel jumlah solusi baku N-Queens (OEIS A000170) untuk reservasi memori cepat
    static const int counts[] = {0, 1, 0, 0, 2, 10, 4, 40, 92, 352, 724, 2680, 14200};
    if (n < static_cast<int>(sizeof(counts) / sizeof(counts[0]))) {
        solutions.reserve(counts[n]);
    }

    std::vector<int> current(n);
    dfsNormal(0, n, 0, 0, 0, current, solutions);
    return solutions;
}

// Kasus 2: Pre-placed N-Queens
std::vector<std::vector<int>> solvePreplaced(int n, const std::vector<std::pair<int, int>>& preplaced) {
    std::vector<std::vector<int>> solutions;
    if (n <= 0) return solutions;

    std::vector<int> fixedCol(n, -1);
    int preplacedColsMask = 0;

    // Validasi awal ratu-ratu yang terkunci
    for (size_t i = 0; i < preplaced.size(); ++i) {
        int r1 = preplaced[i].first, c1 = preplaced[i].second;
        if (r1 < 0 || r1 >= n || c1 < 0 || c1 >= n) return solutions;
        if (fixedCol[r1] != -1) return solutions; // Ada dua ratu di baris yang sama
        if ((preplacedColsMask & (1 << c1)) != 0) return solutions; // Ada dua ratu di kolom yang sama

        fixedCol[r1] = c1;
        preplacedColsMask |= (1 << c1);

        // Cek apakah sesama ratu terkunci saling serang diagonal
        for (size_t j = i + 1; j < preplaced.size(); ++j) {
            int r2 = preplaced[j].first, c2 = preplaced[j].second;
            if (std::abs(r1 - r2) == std::abs(c1 - c2)) {
                return solutions; // Konflik diagonal awal -> 0 solusi
            }
        }
    }

    std::vector<int> current(n);
    dfsPreplaced(0, n, fixedCol, preplacedColsMask, 0, 0, 0, current, solutions);
    return solutions;
}

