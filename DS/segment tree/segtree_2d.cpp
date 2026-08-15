// ============================================================================
// 2D SEGMENT TREE -- point update, rectangle query, O(log n * log m)
// ----------------------------------------------------------------------------
// A segment tree over rows, where every row-node owns a segment tree over
// columns. Iterative on both axes, so it is short; the cost is that it only
// does POINT updates.
//
// ############################################################################
// #  n AND m MUST BE POWERS OF TWO HERE
// #
// #  The 1D iterative tree tolerates any n because each half is still walked
// #  in order. Nesting two of them does not: a row-node then spans two
// #  NON-ADJACENT row ranges and its column tree is meaningless.
// #  Round both up to a power of two and pad with the identity.
// #
// #  MEMORY: 4 * n * m elements. 1024 x 1024 of ll is 33 MB. 2048 x 2048 is
// #  134 MB and out of budget. Check before typing.
// ############################################################################
//
// 0-INDEXED, HALF-OPEN on both axes: [x1, x2) x [y1, y2).
//
// WHY upd() LOOKS DOUBLED: setting one cell fixes its own row's column tree,
//   then walks up the rows, and at each ancestor row rebuilds that row's
//   column tree along the affected column path. Both loops are O(log), so the
//   whole update is O(log n * log m).
//
// PITFALLS:
//   f must be associative AND commutative here -- the row-level fold mixes
//     order. Sum, min, max, gcd, xor are all fine; matrix product is NOT.
//     (The 1D file preserves order; this one does not.)
//   upd() SETS. Read the old value yourself if you want an add.
//   No lazy, so no rectangle updates. For those: 2D BIT on a difference array
//     (DS/BIT/fenwick_2d.cpp), or offline sweep + 1D lazy.
//   Static dimensions -- set n, m before any call.
//
// CHEAPER ALTERNATIVES, check these first:
//   point update + rectangle SUM         DS/BIT/fenwick_2d.cpp -- same
//                                        complexity, a quarter of the memory,
//                                        five lines. Use it unless you need a
//                                        non-invertible f.
//   static grid, no updates              DS/prefix_sum_2d.cpp (sum) or
//                                        DS/sparse/sparse_table_2d.cpp (min/max)
//   sparse points, offline               DS/BIT/fenwick_2d_offline.cpp
//   This file earns its place only for point updates with min/max/gcd -- the
//   ops a Fenwick cannot undo.
// ============================================================================
const int NR = 512, NC = 512;            // powers of two
ll t[2 * NR][2 * NC];
int n = NR, m = NC;                      // active dims, also powers of two
const ll unit = 0;
ll f(ll a, ll b) { return a + b; }       // associative AND commutative

void upd(int x, int y, ll v) {
    int i = x + n, j = y + m;
    t[i][j] = v;
    for (int jj = j >> 1; jj; jj >>= 1) t[i][jj] = f(t[i][jj << 1], t[i][jj << 1 | 1]);
    for (i >>= 1; i; i >>= 1) {
        t[i][j] = f(t[i << 1][j], t[i << 1 | 1][j]);
        for (int jj = j >> 1; jj; jj >>= 1) t[i][jj] = f(t[i][jj << 1], t[i][jj << 1 | 1]);
    }
}
ll qcol(int i, int y1, int y2) {         // row-node i, columns [y1, y2)
    ll res = unit;
    for (int l = y1 + m, r = y2 + m; l < r; l >>= 1, r >>= 1) {
        if (l & 1) res = f(res, t[i][l++]);
        if (r & 1) res = f(res, t[i][--r]);
    }
    return res;
}
ll query(int x1, int y1, int x2, int y2) {   // [x1,x2) x [y1,y2)
    ll res = unit;
    for (int l = x1 + n, r = x2 + n; l < r; l >>= 1, r >>= 1) {
        if (l & 1) res = f(res, qcol(l++, y1, y2));
        if (r & 1) res = f(res, qcol(--r, y1, y2));
    }
    return res;
}
