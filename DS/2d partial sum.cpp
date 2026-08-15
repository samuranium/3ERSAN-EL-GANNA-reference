// ============================================================================
// 2D DIFFERENCE ARRAY -- O(1) range-add, O(n*m) once at the end. OFFLINE.
// ----------------------------------------------------------------------------
// The exact inverse of DS/prefix_sum_2d.cpp. Each rectangle add touches FOUR
// corners; one prefix-sum sweep at the end turns the whole thing into the
// finished grid. Also called the imos method.
//
// ############################################################################
// #  OFFLINE ONLY -- ALL UPDATES, THEN ONE finalize(), THEN READ
// #
// #  d[][] is meaningless until finalize() runs, and finalize() is not
// #  repeatable -- a second call prefix-sums the already-summed grid.
// #  Need updates and queries interleaved? This is the wrong structure:
// #  DS/BIT/fenwick_2d.cpp, or a 2D segment tree.
// ############################################################################
//
// WHY FOUR CORNERS: +v at the top-left says "everything down-right of here
// gains v". That over-applies to three regions, so subtract it back at the row
// below and the column right, then add the doubly-subtracted corner once. Same
// inclusion-exclusion as the prefix query, mirrored.
//
// PITFALLS:
//   Size the array N+2, not N+1 -- add() writes r2+1 and c2+1.
//   1-indexed, same reason as prefix_sum_2d: the zero border removes every
//     boundary check.
//   ll if the adds accumulate; 1e5 updates of 1e9 overflows int immediately.
//   finalize() is the SAME loop as the prefix-sum build. That is not a
//     coincidence: prefix sum and difference are inverse operations.
//
// SUBPROBLEMS:
//   many rectangle adds, read the grid once   this file
//   1D version                                d[l] += v, d[r+1] -= v; one pass
//   "how many rectangles cover each cell"     add 1 per rectangle
//   stamp counting / heat maps                same
//   max overlap depth of rectangles           finalize, then max over the grid
//   range-add on a tree                       diff on the Euler tour -- add at
//                                             tin[u], subtract at tout[u]+1
//   range-add range-sum ONLINE                two Fenwicks, DS/BIT/rangexrange.cpp
// ============================================================================
const int N = 1005;
ll d[N + 2][N + 2];

// add v to every cell of the inclusive rectangle rows [r1,r2], cols [c1,c2]
void add(int r1, int c1, int r2, int c2, ll v) {
    d[r1][c1] += v;
    d[r2 + 1][c1] -= v;
    d[r1][c2 + 1] -= v;
    d[r2 + 1][c2 + 1] += v;
}
// call ONCE after every add(). Afterwards d[i][j] is the true cell value.
void finalize(int n, int m) {
    for (int i = 1; i <= n; i++)
        for (int j = 1; j <= m; j++)
            d[i][j] += d[i - 1][j] + d[i][j - 1] - d[i - 1][j - 1];
}
