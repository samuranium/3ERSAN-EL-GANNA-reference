// ============================================================================
// ITERATIVE SEGMENT TREE -- point update, range query, O(log n), no recursion
// ----------------------------------------------------------------------------
// Bottom-up. Leaves live at t[n .. 2n-1], internal node i has children 2i and
// 2i+1. Half the code of the recursive version and roughly twice the speed --
// this is the one to reach for when you only need point updates.
//
// ############################################################################
// #  WHY TWO ACCUMULATORS AND NOT ONE
// #
// #      if (l & 1) resl = f(resl, t[l++]);
// #      if (r & 1) resr = f(t[--r], resr);
// #
// #  The left walk collects nodes in increasing order, the right walk in
// #  DECREASING order. Folding both into one accumulator applies f out of
// #  order, which is invisible for sum/min and WRONG for matrix product,
// #  string concatenation, or any non-commutative f.
// #  Keeping them apart and joining once at the end preserves the order.
// #
// #  This is also why n need NOT be a power of two: the tree is "rotated" for
// #  other n, but each half is still visited in order.
// ############################################################################
//
// 0-INDEXED, HALF-OPEN query [l, r). query(l, l) is the identity.
//
// NO LAZY. Range updates need the recursive version -- DS/segment tree/lazy.cpp.
//   Range-update/point-query can be faked with a difference array + this.
//
// PITFALLS:
//   `unit` must be the true identity of f: 0 for sum, LLONG_MIN for max,
//     LLONG_MAX for min, 1 for product. A wrong unit is only visible on
//     queries that touch the padding.
//   Half-open, unlike everything else in this folder ([L,R] inclusive there).
//     query(l, r+1) if you are thinking inclusively.
//   upd() SETS, it does not add. Read-modify-write for an add.
//   Sizes: t must hold 2n. build() sets sz, so call it first.
//
// SUBPROBLEMS:
//   range sum / min / max / gcd / xor      swap f and unit
//   range matrix product                   swap f -- order is preserved
//   point update, prefix query             query(0, r)
//   "first index >= l with value >= x"     the recursive descend is easier;
//                                          DS/segment tree/lazy.cpp [1]
//   k-th one in a 0/1 array                DS/BIT/fenwick_kth.cpp is shorter
//   max subarray sum                       custom Node, see lazy.cpp [2]
//   2D version                             DS/segment tree/segtree_2d.cpp
// ============================================================================
const int N = 1 << 18;
ll t[2 * N];
int sz;
const ll unit = 0;                       // identity of f
ll f(ll a, ll b) { return a + b; }       // any ASSOCIATIVE op

void build(int n, ll *a) {
    sz = n;
    for (int i = 0; i < n; i++) t[n + i] = a[i];
    for (int i = n - 1; i > 0; i--) t[i] = f(t[i << 1], t[i << 1 | 1]);
}
void upd(int p, ll v) {                  // SET position p to v
    for (t[p += sz] = v; p >>= 1;) t[p] = f(t[p << 1], t[p << 1 | 1]);
}
ll query(int l, int r) {                 // HALF-OPEN [l, r)
    ll resl = unit, resr = unit;
    for (l += sz, r += sz; l < r; l >>= 1, r >>= 1) {
        if (l & 1) resl = f(resl, t[l++]);
        if (r & 1) resr = f(t[--r], resr);
    }
    return f(resl, resr);
}
