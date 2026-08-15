// ============================================================================
// 2D PREFIX SUMS -- O(n*m) build, O(1) submatrix sum, STATIC grid
// ----------------------------------------------------------------------------
// p[i][j] = sum of the rectangle from (1,1) to (i,j). Build in place: seed p
// with the raw values, then one sweep.
//
// ############################################################################
// #  1-INDEXED ON PURPOSE
// #  Row 0 and column 0 are the zero border. They are what make both the build
// #  and the query work with no boundary check -- p[i-1][*] and p[*][j-1] are
// #  always in range and always 0 at the edge.
// #  Going 0-indexed means guarding four subtractions instead. Do not.
// ############################################################################
//
// THE SIGNS: query subtracts the two overhanging strips, which removes their
// shared corner twice, so it is added back. That is inclusion-exclusion over
// 2 dimensions -- 2^2 = 4 terms. In 3D it is 8 terms with signs by parity of
// how many coordinates you stepped back; in d dimensions, 2^d.
//
// PITFALLS:
//   ll unless you are certain: 1e3 x 1e3 cells of 1e9 is 1e15.
//   query with r1 > r2 or c1 > c2 returns garbage, not 0 -- guard upstream.
//   Static only. Any cell update means an O(n*m) rebuild -> DS/BIT/fenwick_2d.cpp.
//   Build is in place, so calling build() twice doubles everything.
//
// SUBPROBLEMS:
//   count of 1s / of a colour in a rect  build over an indicator grid
//   max-sum submatrix                    fix the top and bottom rows, collapse
//                                        each column to one number, Kadane over
//                                        it. O(n^2 * m).
//   # submatrices with sum == k          fix the row pair, then it is the 1D
//                                        "count subarrays with sum k" hash map
//   is a submatrix all-equal             sum == value * area, plus a second
//                                        prefix over squares to be safe
//   largest all-ones square              DP, not this -- but the O(1) rect sum
//                                        makes a binary search version trivial
//   3D version                           p[i][j][k] with 8 inclusion-exclusion
//                                        terms; same shape, sign = parity
// ============================================================================
const int N = 1005;
ll p[N][N];

// seed p[1..n][1..m] with the raw values first, then call this ONCE
void build(int n, int m) {
    for (int i = 1; i <= n; i++)
        for (int j = 1; j <= m; j++)
            p[i][j] += p[i - 1][j] + p[i][j - 1] - p[i - 1][j - 1];
}
// sum of the inclusive rectangle rows [r1, r2], cols [c1, c2]
ll query(int r1, int c1, int r2, int c2) {
    return p[r2][c2] - p[r1 - 1][c2] - p[r2][c1 - 1] + p[r1 - 1][c1 - 1];
}
