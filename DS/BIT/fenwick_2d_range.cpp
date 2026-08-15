// ============================================================================
// 2D FENWICK, RANGE UPDATE + RANGE QUERY -- four trees, O(log n log m)
// ----------------------------------------------------------------------------
// The 2D analogue of DS/BIT/rangexrange.cpp. A rectangle add is four point
// updates on a 2D difference array; recovering a prefix sum from that array
// needs the coefficients i*j, i, j and 1 tracked separately -- hence four
// Fenwicks rather than one.
//
//   S(x,y) = x*y*T0(x,y) - y*T1(x,y) - x*T2(x,y) + T3(x,y)
//
//   T0 = sum d          T1 = sum d*(i-1)
//   T2 = sum d*(j-1)    T3 = sum d*(i-1)*(j-1)
//
// ############################################################################
// #  WHY FOUR AND NOT ONE
// #
// #  In 1D, a difference-array prefix sum needs sum(d) scaled by x, minus
// #  sum(d*(i-1)) -- two trees. In 2D the product x*y expands to four terms,
// #  one per subset of {i, j}. In d dimensions it is 2^d trees, which is why
// #  nobody does this past 2D.
// #
// #  Using one tree and "fixing it up" does not work: the coefficients depend
// #  on WHERE the update was, and a single tree cannot remember that.
// ############################################################################
//
// PITFALLS:
//   1-INDEXED and the arrays must be (n+2) x (m+2) -- upd() writes x2+1 and
//     y2+1.
//   ll everywhere; the (i-1)*(j-1) products are large before the sums even
//     start.
//   Four trees is 4*n*m cells. At 1000x1000 that is 32 MB of ll. If you only
//     need range-update / POINT-query, one tree suffices -- see below.
//   Multi-test: clear all four over (n+2)*(m+2) only.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   rectangle add, rectangle sum        this file
//   rectangle add, POINT query          ONE tree: add at the four corners of a
//                                       2D difference array, and a point query
//                                       is its prefix sum. Quarter the memory.
//   point add, rectangle sum            DS/BIT/fenwick_2d.cpp -- one tree
//   static grid, no updates             DS/prefix_sum_2d.cpp -- no tree at all
//   offline: all updates, then read     DS/2d partial sum.cpp -- one sweep
//   rectangle XOR, rectangle XOR-query  the same four-tree idea but the
//                                       coefficients collapse: xor is its own
//                                       inverse, so parity of (i-1)(j-1)
//                                       decides which tree, and no
//                                       multiplication is needed
//   count points in a rectangle         point add of 1, this or fenwick_2d
//
// NOT THIS: sparse coordinates -> DS/BIT/fenwick_2d_offline.cpp. min/max
//   instead of sum -> a Fenwick cannot undo those; use a 2D segment tree.
// ============================================================================
const int FN2 = 1005, FM2 = 1005;
ll T[4][FN2 + 2][FM2 + 2];
int fn, fm;                                  // set before use

void add_at(int b, int x, int y, ll v) {
    for (int i = x; i <= fn + 1; i += i & -i)
        for (int j = y; j <= fm + 1; j += j & -j) T[b][i][j] += v;
}
// internal: the four coefficient updates for a single difference-array point
void diff_point(int x, int y, ll v) {
    add_at(0, x, y, v);
    add_at(1, x, y, v * (x - 1));
    add_at(2, x, y, v * (y - 1));
    add_at(3, x, y, v * (x - 1) * (y - 1));
}
// add v to every cell of the inclusive rectangle [x1,x2] x [y1,y2]
void upd(int x1, int y1, int x2, int y2, ll v) {
    diff_point(x1, y1, v);
    diff_point(x1, y2 + 1, -v);
    diff_point(x2 + 1, y1, -v);
    diff_point(x2 + 1, y2 + 1, v);
}
ll read(int b, int x, int y) {
    ll s = 0;
    for (int i = x; i > 0; i -= i & -i)
        for (int j = y; j > 0; j -= j & -j) s += T[b][i][j];
    return s;
}
ll pre(int x, int y) {                       // sum over [1..x] x [1..y]
    return read(0, x, y) * x * y - read(1, x, y) * y - read(2, x, y) * x
         + read(3, x, y);
}
ll query(int x1, int y1, int x2, int y2) {
    return pre(x2, y2) - pre(x1 - 1, y2) - pre(x2, y1 - 1) + pre(x1 - 1, y1 - 1);
}
