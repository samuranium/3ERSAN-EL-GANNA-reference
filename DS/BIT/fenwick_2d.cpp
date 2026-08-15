// ============================================================================
// 2D FENWICK -- point update, submatrix sum, O(log n * log m) both
// ----------------------------------------------------------------------------
// A Fenwick of Fenwicks. Both loops are the plain 1D walk; nesting them is the
// entire construction.
//
// ############################################################################
// #  THE GRID MUST BE DENSE
// #
// #  Memory is n * m cells, ALWAYS -- 1e5 x 1e5 is not 1e5 points, it is 1e10
// #  cells and it will not fit. This structure is for grids up to about
// #  2000 x 2000.
// #
// #  Sparse points on a huge coordinate range -> DS/BIT/fenwick_2d_offline.cpp
// #  (if you can see all updates first) or a merge-sort tree / BIT of sorted
// #  vectors. Coordinate compression alone does NOT save you here: n compressed
// #  x-values times m compressed y-values is still n*m cells.
// ############################################################################
//
// 1-INDEXED, and it must be: i += i & -i never terminates from 0.
//
// PITFALLS:
//   ll for sums. 4e6 cells of 1e9 is 4e15.
//   upd() ADDS a delta. To SET a cell you need its old value: keep the raw
//     grid alongside and call upd(x, y, newval - grid[x][y]).
//   Multi-test: memset over (n+1)*(m+1) only, not the whole N*N -- clearing
//     1e6 cells per case is what makes these TLE.
//
// SUBPROBLEMS:
//   point add, rectangle sum            this file
//   point SET, rectangle sum            keep grid[][], add the delta
//   count points in a rectangle         upd(x, y, 1)
//   rectangle add, point query          run it on a 2D difference array:
//                                       add v at (r1,c1), -v at (r2+1,c1) and
//                                       (r1,c2+1), +v at (r2+1,c2+1); then a
//                                       point query is a prefix sum
//   rectangle add, rectangle sum        four 2D Fenwicks with the
//                                       i*j / i / j / 1 coefficients -- the 2D
//                                       analogue of DS/BIT/rangexrange.cpp
//   offline / sparse coordinates        fenwick_2d_offline.cpp
// ============================================================================
const int N = 1005, M = 1005;
ll t[N][M];
int n, m;                       // set before use

void upd(int x, int y, ll v) {
    for (int i = x; i <= n; i += i & -i)
        for (int j = y; j <= m; j += j & -j) t[i][j] += v;
}
ll pre(int x, int y) {          // sum over [1..x] x [1..y]
    ll s = 0;
    for (int i = x; i > 0; i -= i & -i)
        for (int j = y; j > 0; j -= j & -j) s += t[i][j];
    return s;
}
// inclusive rectangle rows [r1,r2], cols [c1,c2]
ll query(int r1, int c1, int r2, int c2) {
    return pre(r2, c2) - pre(r1 - 1, c2) - pre(r2, c1 - 1) + pre(r1 - 1, c1 - 1);
}
