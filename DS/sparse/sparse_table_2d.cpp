// ============================================================================
// 2D SPARSE TABLE -- O(n*m*log n*log m) build, O(1) rectangle query
//                    STATIC matrix, IDEMPOTENT op only
// ----------------------------------------------------------------------------
// t[ki][kj][i][j] = op over the block of height 2^ki and width 2^kj whose top
// left corner is (i, j). A query covers the rectangle with FOUR overlapping
// blocks -- one anchored at each corner -- which is why the op must be
// idempotent: min, max, gcd, AND, OR. Never sum.
//
// ############################################################################
// #  MEMORY IS THE BINDING CONSTRAINT, NOT TIME
// #
// #      LG * LG * N * N * sizeof(elem)
// #
// #  At N = 305, LG = 9, int: ~30 MB. At N = 500, LG = 10 it is ~100 MB and
// #  you are out of budget on most judges. Check this BEFORE you type it.
// #  If n*m is large but queries are few, a plain O(rows*cols) scan per query
// #  often wins outright.
// ############################################################################
//
// 1-INDEXED, inclusive rectangle rows [r1,r2] cols [c1,c2].
//
// BUILD ORDER MATTERS: grow columns first at ki = 0, then grow rows on top of
//   the finished column tables. Doing rows first and columns after works too,
//   but mixing the two inside one loop reads half-built entries.
//
// PITFALLS:
//   LG must satisfy (1 << LG) > max(n, m).
//   __lg(0) is undefined -- guard r1 <= r2 and c1 <= c2 upstream.
//   Idempotent only. For sum use DS/prefix_sum_2d.cpp (O(1), static) or
//     DS/BIT/fenwick_2d.cpp (with updates).
//   Static only.
//
// SUBPROBLEMS:
//   min / max over any submatrix          this
//   gcd over a submatrix                  this
//   is a k x k square uniform             min == max over it, one query each
//   largest square with max-min <= x      binary search the side + this
//   sliding k x k window over a grid      this, or two passes of a monotonic
//                                         deque (O(n*m), far less memory)
// ============================================================================
const int N = 305, LG = 9;
int t[LG][LG][N][N], a[N][N];

int op(int x, int y) { return min(x, y); }   // idempotent only

void build(int n, int m) {
    for (int i = 1; i <= n; i++)
        for (int j = 1; j <= m; j++) t[0][0][i][j] = a[i][j];
    for (int kj = 1; (1 << kj) <= m; kj++)                       // widen first
        for (int i = 1; i <= n; i++)
            for (int j = 1; j + (1 << kj) - 1 <= m; j++)
                t[0][kj][i][j] = op(t[0][kj - 1][i][j],
                                    t[0][kj - 1][i][j + (1 << (kj - 1))]);
    for (int ki = 1; (1 << ki) <= n; ki++)                       // then heighten
        for (int kj = 0; (1 << kj) <= m; kj++)
            for (int i = 1; i + (1 << ki) - 1 <= n; i++)
                for (int j = 1; j + (1 << kj) - 1 <= m; j++)
                    t[ki][kj][i][j] = op(t[ki - 1][kj][i][j],
                                         t[ki - 1][kj][i + (1 << (ki - 1))][j]);
}
// inclusive rectangle, 1 <= r1 <= r2 <= n, 1 <= c1 <= c2 <= m
int query(int r1, int c1, int r2, int c2) {
    int ki = __lg(r2 - r1 + 1), kj = __lg(c2 - c1 + 1);
    int i2 = r2 - (1 << ki) + 1, j2 = c2 - (1 << kj) + 1;
    return op(op(t[ki][kj][r1][c1], t[ki][kj][i2][c1]),
              op(t[ki][kj][r1][j2], t[ki][kj][i2][j2]));
}
