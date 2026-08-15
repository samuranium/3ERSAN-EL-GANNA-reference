// ============================================================================
// hash_2d.h  —  2D rolling hash for GRIDS.  mod 2^61-1.
// ============================================================================
//
// WHAT THIS FILE DOES
//   O(1) hash of any axis-aligned rectangle of a grid, after O(nm) setup.
//   From that: 2D pattern search, counting distinct h x w submatrices,
//   largest common square of two grids, and mirror / rotation tests (hash the
//   flipped grid and compare rectangles across the two).
//
//   Works on char grids AND on integer grids with values up to 1e9 — pass
//   vector<vector<int>> instead of vector<string>, no other edit.
//
// ----------------------------------------------------------------------------
// TYPES — what must be what, and why
// ----------------------------------------------------------------------------
//   `#define int long long` + `int32_t main()`.
//
//   long long is ENOUGH for hash values: they live in [0, 2^61-2], so
//     add()'s intermediate a+b < 2^62 < 2^63-1.
//   __int128 is REQUIRED in mul(): 61-bit x 61-bit = 122-bit product.
//   unsigned long long is REQUIRED in SplitMix and nowhere else. If you drop
//     the distinct/largest-common-square helpers, you can delete `ull`.
//
//   MEMORY: P is (n+1) x (m+1) of 8-byte entries. At 2000x2000 that is
//     ~32 MB for ONE grid. Two grids (largest_common_square) is ~64 MB.
//     This is the one place in the set where the 8-byte cost actually bites;
//     if you are near the limit, shrink the grid, not the type — the type is
//     load-bearing.
//
// ----------------------------------------------------------------------------
// MINIMAL BUILDS — when you need X, copy Y
// ----------------------------------------------------------------------------
//   CORE (always, ~35 lines):
//     MOD, BASE_ROW, BASE_COL, pw_row[], ipw_row[], pw_col[], ipw_col[],
//     add, sub, mul, power, enc, init_hash
//
//   compare two rectangles .............. CORE + Hash2D{build, rect}   ~25
//   2D pattern search ................... the above + whole, find_pattern +10
//   # distinct h x w submatrices ........ the above + SplitMix, HashSet,
//                                         distinct_rect                 +10
//   largest common square ............... the above + largest_common_square +18
//   mirror / rotation checks ............ CORE + Hash2D + flip_rows,
//                                         flip_cols; build a second Hash2D
//                                         on the flipped grid            +8
//
//   You never need a second modulus lane here. See the next block.
//
// ----------------------------------------------------------------------------
// THE MODEL
// ----------------------------------------------------------------------------
//   Exponents factor across the two axes, so a 2D hash is just a 2D partial
//   sum and normalising is one multiply per axis:
//
//     P[i][j] = sum over r<=i, c<=j of  enc(a[r][c]) * BR^(r-1) * BC^(c-1)
//
//     rect(r1,c1,r2,c2) = ( P[r2][c2] - P[r1-1][c2] - P[r2][c1-1]
//                                     + P[r1-1][c1-1] ) * BR^-(r1-1) * BC^-(c1-1)
//
//   Inclusion-exclusion to cut the rectangle out, then slide its top-left
//   corner to (1,1) so equal rectangles hash equally wherever they sit.
//
// WHY 2^61-1 MATTERS MOST HERE
//   A 1000x1000 grid has ~1e6 rectangles of a given size. One lane mod 1e9+7
//   gives a collision probability around 1e12 / 2e9 — collisions are certain.
//   Mod 2^61-1 it is about 4e-7. This is why the two-lane version of this
//   file existed; single lane mod 2^61-1 removes the need entirely.
//
// ----------------------------------------------------------------------------
// QUICK INDEX (1-indexed inclusive)
// ----------------------------------------------------------------------------
//   init_hash(MAXN - 1)             once in main
//   Hash2D H(grid);                 vector<string> or vector<vector<int>>
//
//   H.rect(r1, c1, r2, c2)          hash of that rectangle             O(1)
//   H.whole()                       hash of the entire grid            O(1)
//   find_pattern(text, pat)         all top-left corners of pat        O(nm)
//   distinct_rect(H, h, w)          # distinct h x w submatrices   O(nm) exp
//   largest_common_square(A, B)     side of the largest shared square
//                                                              O(nm log) exp
//   flip_rows(a) / flip_cols(a)     mirrored grid, to hash separately
//
// SIZING
//   MAXN must be >= max(rows, cols) + 2. MAXN = 2005 covers 2000x2000.
//   init_hash(MAXN - 1), never init_hash(MAXN).
// ============================================================================

#include <bits/stdc++.h>
using namespace std;

#define int long long
#define el '\n'
#define ull unsigned long long   // ONLY for SplitMix; see TYPES above
#define fio ios_base::sync_with_stdio(0), cin.tie(0)

const int MAXN = 2005;             // >= max(rows, cols) + 2
const int MOD  = (1LL << 61) - 1;

int BASE_ROW, BASE_COL;
int pw_row[MAXN], ipw_row[MAXN];   // BR^i and BR^-i
int pw_col[MAXN], ipw_col[MAXN];   // BC^i and BC^-i

// a + b in the field. Safe in signed: a, b < 2^61 so a + b < 2^62.
int add(int a, int b) {
    a += b;
    return a >= MOD ? a - MOD : a;
}

int sub(int a, int b) {
    return add(a, MOD - b);
}

// a * b mod 2^61-1: 2^61 == 1 here, so high bits fold onto low bits.
// __int128 is REQUIRED — the product is up to 122 bits.
int mul(int a, int b) {
    __int128 c = (__int128)a * b;
    int lo = (int)(c & MOD);
    int hi = (int)(c >> 61);
    lo += hi;
    return lo >= MOD ? lo - MOD : lo;
}

int power(int b, int e) {
    int r = 1;
    for (; e; b = mul(b, b), e >>= 1)
        if (e & 1) r = mul(r, b);
    return r;
}

// Encode a raw cell value into [1, MOD-1]. Never 0.
int enc(int x) {
    x %= MOD - 1;
    if (x < 0) x += MOD - 1;
    return x + 1;
}

// Call once in main. n = max(rows, cols). Two independent random bases —
// they must differ, or a row shift and a column shift become confusable.
void init_hash(int n) {
    mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count()
                   ^ (ull)(uintptr_t)&n);
    BASE_ROW = (int)(rng() % (ull)(MOD - 4)) + 2;
    do { BASE_COL = (int)(rng() % (ull)(MOD - 4)) + 2; } while (BASE_COL == BASE_ROW);

    int inv_row = power(BASE_ROW, MOD - 2);
    int inv_col = power(BASE_COL, MOD - 2);
    pw_row[0] = ipw_row[0] = pw_col[0] = ipw_col[0] = 1;

    for (int i = 1; i <= n; i++) {
        pw_row[i]  = mul(pw_row[i - 1], BASE_ROW);
        ipw_row[i] = mul(ipw_row[i - 1], inv_row);
        pw_col[i]  = mul(pw_col[i - 1], BASE_COL);
        ipw_col[i] = mul(ipw_col[i - 1], inv_col);
    }
}

// splitmix64 for unordered_set. ull is mandatory inside: wraparound at 2^64
// and a logical right shift.
struct SplitMix {
    size_t operator()(int x) const {
        ull z = (ull)x + 0x9e3779b97f4a7c15;
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
        z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
        return (size_t)(z ^ (z >> 31));
    }
};
using HashSet = unordered_set<int, SplitMix>;

// ----------------------------------------------------------------------------
// Hash2D — static grid, O(1) rectangle hash.
// ----------------------------------------------------------------------------
struct Hash2D {
    int n, m;
    vector<vector<int>> P;  // weighted 2D prefix sum

    Hash2D() {}

    Hash2D(const vector<string> &g) {
        vector<vector<int>> v(g.size());
        for (size_t i = 0; i < g.size(); i++)
            for (char c : g[i]) v[i].push_back((unsigned char)c);
        build(v);
    }

    Hash2D(const vector<vector<int>> &v) { build(v); }

    void build(const vector<vector<int>> &a) {
        n = a.size();
        m = n ? a[0].size() : 0;
        P.assign(n + 1, vector<int>(m + 1, 0));
        for (int i = 1; i <= n; i++)
            for (int j = 1; j <= m; j++) {
                int cell = mul(enc(a[i - 1][j - 1]),
                               mul(pw_row[i - 1], pw_col[j - 1]));
                P[i][j] = sub(add(add(P[i - 1][j], P[i][j - 1]), cell),
                              P[i - 1][j - 1]);
            }
    }

    // Inclusion-exclusion for the rectangle, then divide out the row and
    // column offsets so the top-left corner sits at exponent (0, 0).
    int rect(int r1, int c1, int r2, int c2) const {
        if (r1 > r2 || c1 > c2) return 0;
        int v = sub(add(P[r2][c2], P[r1 - 1][c1 - 1]),
                    add(P[r1 - 1][c2], P[r2][c1 - 1]));
        return mul(v, mul(ipw_row[r1 - 1], ipw_col[c1 - 1]));
    }

    int whole() const { return rect(1, 1, n, m); }
};

// All top-left corners of the text grid where the pattern grid sits exactly.
// One O(1) rectangle hash per candidate position.
vector<pair<int, int>> find_pattern(const Hash2D &text, const Hash2D &pat) {
    vector<pair<int, int>> res;
    int h = pat.n, w = pat.m;
    if (h > text.n || w > text.m) return res;

    int target = pat.whole();
    for (int i = 1; i + h - 1 <= text.n; i++)
        for (int j = 1; j + w - 1 <= text.m; j++)
            if (text.rect(i, j, i + h - 1, j + w - 1) == target)
                res.push_back({i, j});
    return res;
}

// Number of distinct h x w submatrices: dump every window into a set.
int distinct_rect(const Hash2D &A, int h, int w) {
    HashSet s;
    for (int i = 1; i + h - 1 <= A.n; i++)
        for (int j = 1; j + w - 1 <= A.m; j++)
            s.insert(A.rect(i, j, i + h - 1, j + w - 1));
    return s.size();
}

// Largest k such that A and B share a common k x k square.
// "a common square of side k exists" is monotone in k, so bisect on k; test
// one k by hashing all of A's squares into a set and probing B's.
int largest_common_square(const Hash2D &A, const Hash2D &B) {
    int lo = 0, hi = min(min(A.n, A.m), min(B.n, B.m));

    auto feasible = [&](int k) {
        HashSet s;
        for (int i = 1; i + k - 1 <= A.n; i++)
            for (int j = 1; j + k - 1 <= A.m; j++)
                s.insert(A.rect(i, j, i + k - 1, j + k - 1));
        for (int i = 1; i + k - 1 <= B.n; i++)
            for (int j = 1; j + k - 1 <= B.m; j++)
                if (s.count(B.rect(i, j, i + k - 1, j + k - 1))) return true;
        return false;
    };

    while (lo < hi) {
        int m = (lo + hi + 1) >> 1;
        if (feasible(m)) lo = m;
        else hi = m - 1;
    }
    return lo;
}

// For mirror / rotation questions: build a second Hash2D on the flipped grid
// and compare rectangles across the two.
vector<vector<int>> flip_rows(vector<vector<int>> a) {
    reverse(a.begin(), a.end());
    return a;
}

vector<vector<int>> flip_cols(vector<vector<int>> a) {
    for (auto &row : a) reverse(row.begin(), row.end());
    return a;
}

int32_t main() {
    fio;
    init_hash(MAXN - 1);   // shrink MAXN when n is small

    return 0;
}
