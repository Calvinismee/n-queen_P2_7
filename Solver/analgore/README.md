# N-Queens Backtracking Solver

Solver N-Queens dengan algoritma **Backtracking + Bitwise Pruning** untuk 4 kasus berbeda. Semua solver dikompilasi ke WebAssembly dan dijalankan di dalam tester.

## Koreksi dan batas masukan

Implementasi menerima N=1 sampai 12; antarmuka tetap membatasi N=4 sampai 12. Nilai di luar rentang, koordinat ratu terkunci di luar papan, pengulangan/konflik ratu terkunci, ukuran grid yang tidak N×N, serta karakter selain `.`/`H` atau `.`/`W` ditolak sebelum akses indeks dan operasi bit. Penolakan mengembalikan daftar kosong, sehingga pemanggil tetap perlu memvalidasi masukan untuk membedakannya dari persoalan sah tanpa solusi.

Kasus Walled dengan seluruh sel `W` mempunyai satu susunan optimal: papan tanpa ratu. Hasilnya adalah satu daftar posisi kosong (`{{}}`), bukan tidak mengembalikan papan (`{}`).

Pada Debug run, rekaman mencakup penempatan dan langkah mundur. Baris bebas menggunakan `-1` dan dibersihkan ketika mundur agar tidak ada ratu sisa pada rekaman. Ratu terkunci tetap terlihat. Rekaman mengikuti batas kepala/ekor tester; pencarian panjang tidak menyimpan seluruh langkah.

Jalankan `npm test`, `npm run test:native`, dan `npm run build` dari akar proyek. Pengujian native memakai AddressSanitizer/UndefinedBehaviorSanitizer, enumerasi permutasi terpisah untuk Pre-placed/Holes, dan enumerasi subset sel dengan pemeriksaan delapan arah untuk Walled. Konfigurasi bawaan memakai solver contoh brute force sebagai pembanding; ketika pembanding tidak selesai, hasil diberi status **Validity Only**, sehingga kelengkapan belum diklaim.

---

## Konsep Dasar: Backtracking

Backtracking adalah teknik pencarian solusi yang menjelajahi semua kemungkinan secara rekursif, lalu **memangkas cabang (pruning)** begitu ditemukan pelanggaran constraint — tanpa perlu meneruskan pencarian ke bawah cabang tersebut.

Pada N-Queens, pencarian dilakukan **baris per baris** (baris 0 sampai N-1). Setiap baris ditempatkan tepat satu ratu pada kolom yang aman. Jika di baris tertentu tidak ada kolom yang aman, algoritma mundur (*backtrack*) ke baris sebelumnya dan mencoba kolom lain.

### Representasi Bitmask

Ketiga constraint (kolom, diagonal kiri, diagonal kanan) direpresentasikan sebagai **bitmask integer**:

| Mask | Arti |
|------|------|
| `cols` | Bit ke-c aktif → kolom c sudah terisi ratu |
| `d1` | Diagonal serong kiri (`r - c`) yang terancam |
| `d2` | Diagonal serong kanan (`r + c`) yang terancam |

Kolom yang **aman** di baris saat ini dihitung dalam satu operasi bitwise:

```cpp
int safe = (~(cols | d1 | d2)) & ((1 << n) - 1);
```

Setiap bit yang menyala di `safe` adalah kolom valid. Bit paling kanan diambil dengan `bit & -bit`, dan indeks kolomnya didapat via `__builtin_ctz(bit)`.

---

## Kasus 1 — Normal (`normal.cpp`)

**Tujuan:** Tempatkan N ratu di papan N×N kosong. Kembalikan semua susunan valid.

**Aturan:** Tidak ada dua ratu yang boleh berbagi baris, kolom, atau diagonal.

### Algoritma

```
dfs(baris r):
    jika r == n → simpan solusi
    hitung kolom aman di baris r (bitmask)
    untuk setiap kolom aman c:
        tempatkan ratu di (r, c)
        perbarui cols, d1, d2
        rekursi ke baris r+1
        (otomatis backtrack saat rekursi selesai)
```

### Snippet Kunci

```cpp
void dfs(int r, int n, int cols, int d1, int d2, ...) {
    if (r == n) { sols.push_back(cur); return; }

    int safe = (~(cols | d1 | d2)) & ((1 << n) - 1);
    while (safe) {
        int bit = safe & -safe;          // ambil bit paling kanan
        cur[r] = __builtin_ctz(bit);     // indeks kolom
        dfs(r + 1, n,
            cols | bit,                  // kolom c terancam
            (d1 | bit) << 1,             // diagonal kiri geser kiri tiap baris
            (d2 | bit) >> 1,             // diagonal kanan geser kanan tiap baris
            cur, sols);
        safe &= safe - 1;                // hapus bit yang sudah dicoba
    }
}
```

Pembaruan `d1 << 1` dan `d2 >> 1` adalah kunci — ancaman diagonal "bergeser" satu posisi setiap turun satu baris, sehingga proyeksi diagonal tetap akurat tanpa perlu iterasi sel.

---

## Kasus 2 — Pre-placed (`preplaced.cpp`)

**Tujuan:** N ratu ditempatkan, sebagian sudah terkunci di posisi tertentu. Temukan semua susunan valid yang mempertahankan ratu terkunci tersebut.

**Aturan:** Sama seperti Normal, ditambah ratu terkunci wajib ada di posisi aslinya.

### Algoritma

Sebelum DFS, dilakukan **validasi awal**:
- Tidak boleh ada dua ratu terkunci di baris atau kolom yang sama.
- Tidak boleh ada dua ratu terkunci yang saling menyerang diagonal.

Saat DFS, tiap baris punya dua cabang:

```
jika baris r punya ratu terkunci:
    cek apakah posisinya terancam oleh ratu di atas
    jika aman → tempatkan langsung, lanjut ke r+1
    jika terancam → pangkas (prune)
jika baris r bebas:
    kolom yang boleh dipilih = aman & bukan kolom ratu terkunci mana pun
```

### Snippet Kunci

```cpp
if (fixed[r] >= 0) {
    int bit = 1 << fixed[r];
    if ((cols | d1 | d2) & bit) return;   // ratu terkunci terancam → pruning
    cur[r] = fixed[r];
    dfs(r + 1, ...);
} else {
    // pCols: bitmask semua kolom yang dimiliki ratu terkunci
    int safe = (~(cols | d1 | d2 | pCols)) & ((1 << n) - 1);
    while (safe) { /* coba tiap kolom aman */ }
}
```

`pCols` memastikan baris bebas tidak "mencuri" kolom yang sudah dipesan ratu terkunci di baris lain.

---

## Kasus 3 — Holes (`holes.cpp`)

**Tujuan:** Papan memiliki lubang (`H`). Ratu tidak boleh berdiri di atas lubang. Serangan tetap **menembus** lubang. Temukan semua susunan valid.

**Aturan:** Sama seperti Normal, ditambah larangan penempatan di sel berlubang.

### Algoritma

Perbedaan dari Normal hanya pada **set kolom yang boleh dipilih** di tiap baris. Dibuat `freeMask[r]` — bitmask kolom bebas (non-lubang) untuk tiap baris — saat preprocessing.

```
preprocessing:
    freeMask[r] = bitmask semua kolom di baris r yang bukan 'H'

dfs:
    safe = (~(cols | d1 | d2)) & freeMask[r] & ((1<<n)-1)
```

Dengan AND `freeMask[r]`, kolom berlubang otomatis tidak muncul di `safe` tanpa logika tambahan.

### Snippet Kunci

```cpp
// Preprocessing: bangun freeMask
for (int r = 0; r < n; ++r)
    for (int c = 0; c < n; ++c)
        if (grid[r][c] != 'H') fMask[r] |= (1 << c);

// DFS: satu baris tambah AND dengan freeMask
int safe = (~(cols | d1 | d2)) & fMask[r] & ((1 << n) - 1);
```

Karena serangan menembus lubang, `d1` dan `d2` tetap diperbarui persis seperti kasus Normal — tidak ada perbedaan logika diagonal.

---

## Kasus 4 — Walled (`walled.cpp`)

**Tujuan:** Papan memiliki tembok (`W`). Tembok **memblokir serangan**. Tujuan bukan N ratu, melainkan **sebanyak mungkin ratu**. Kembalikan semua susunan dengan jumlah ratu terbanyak.

**Aturan:**
- Ratu tidak boleh di atas tembok.
- Serangan berhenti di tembok (tidak menembus).
- Satu baris bisa punya lebih dari satu ratu jika dipisahkan tembok.

### Algoritma

Karena satu baris bisa menampung beberapa ratu (tiap segmen bebas antara dua tembok), representasi `cols[r]` tidak lagi cukup. Solusi disimpan sebagai daftar `(row, col)`.

Struktur `Search` bekerja baris per baris dengan tiga komponen:

**1. Preprocessing — `rowMax`**

Hitung jumlah segmen bebas tiap baris. Ini digunakan sebagai **upper bound** sumbangan ratu per baris.

```cpp
// hitung jumlah segmen (stretch bebas) per baris
int segs = 0, c = 0;
while (c < n) {
    if (grid[r][c] == 'W') { c++; continue; }
    segs++;
    while (c < n && grid[r][c] != 'W') c++;
}
rowMax[r] = segs;
```

**2. Pembangkit opsi baris — `gen()`**

Untuk tiap baris, bangkitkan semua kombinasi penempatan yang mungkin: tiap segmen bebas bisa kosong atau berisi tepat satu ratu.

```cpp
// untuk tiap segmen: pilih kosong, atau salah satu sel di segmen
gen(r, c, picked, opts);           // pilih kosong
for (int i = start; i < c; ++i) { // atau pilih sel i
    picked.push_back(i);
    gen(r, c, picked, opts);
    picked.pop_back();
}
```

**3. DFS + Branch and Bound**

```cpp
void dfs(int r, int count) {
    // bound: maksimum ratu yang masih bisa ditambah dari baris r ke bawah
    int bound = 0;
    for (int i = r; i < n; ++i) bound += rowMax[i];
    if (count + bound < (int)bestN) return;  // pruning

    if (r == n) { /* update best */ return; }

    // coba tiap kombinasi untuk baris r, besar dulu (greedy ordering)
    std::sort(opts, besar_dulu);
    for (auto& cols : opts) {
        if (!compatible(cols)) continue;   // cek serangan dengan ratu yang sudah ada
        letakkan(cols); dfs(r+1, ...); ambil_balik(cols);
    }
}
```

**Cek serangan (`hits`)** menelusuri sel satu per satu antara dua ratu. Jika ada `'W'` di jalurnya, serangan terblokir.

```cpp
bool hits(int r1, int c1, int r2, int c2) {
    // pastikan segaris (baris, kolom, atau diagonal)
    int dr = sign(r2-r1), dc = sign(c2-c1);
    for (int r=r1+dr, c=c1+dc; r!=r2||c!=c2; r+=dr, c+=dc)
        if (grid[r][c] == 'W') return false;  // tembok memblokir
    return true;
}
```

---

## Perbandingan Singkat

| Kasus | Constraint tambahan | Teknik khusus | Return type |
|-------|--------------------|--------------------|-------------|
| Normal | — | Bitwise bitmask | `vector<vector<int>>` |
| Pre-placed | Ratu terkunci di baris tertentu | Validasi awal + `pCols` mask | `vector<vector<int>>` |
| Holes | Sel tertentu tidak bisa diisi | `freeMask` per baris | `vector<vector<int>>` |
| Walled | Tembok blokir serangan; maks ratu | Branch and bound + segment gen | `vector<vector<pair<int,int>>>` |

Kasus 1–3 mengembalikan **satu kolom per baris** (`solution[r]` = kolom ratu di baris `r`). Kasus 4 mengembalikan **daftar posisi `(row, col)`** karena satu baris bisa kosong atau berisi lebih dari satu ratu.
