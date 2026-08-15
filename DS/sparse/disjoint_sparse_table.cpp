// ============================================================================
// DISJOINT SPARSE TABLE -- O(n log n) build, O(1) query, ANY associative op
// ----------------------------------------------------------------------------
// The answer to "I want O(1) range queries but my operation is not idempotent."
// A normal sparse table (DS/sparse/Sparse Table.cpp) reads two OVERLAPPING
// blocks, which is fine for min/max/gcd and silently wrong for sum, product or
// xor. This one reads two DISJOINT blocks that meet exactly at a split point,
// so every element is folded in exactly once.
//
// ############################################################################
// #  HOW THE QUERY FINDS THE SPLIT -- the whole trick in one line
// #
// #      int k = __lg(l ^ r);   return op(t[k][l], t[k][r]);
// #
// #  The highest bit where l and r DIFFER identifies the unique level whose
// #  block boundary falls strictly between them. On that level t[k][l] holds
// #  the fold of [l .. mid) and t[k][r] the fold of [mid .. r]. Disjoint,
// #  adjacent, in order. No overlap, so associativity is all you need.
// #
// #  l == r must be special-cased: l ^ r == 0 and __lg(0) is undefined.
// ############################################################################
//
// 0-INDEXED, inclusive [l, r].
//
// ORDER IS PRESERVED: op(t[k][l], t[k][r]) folds the left part before the
//   right part, so NON-COMMUTATIVE ops work -- matrix product, function
//   composition, "assign then add" transforms. Do not swap the arguments.
//
// PITFALLS:
//   LG must satisfy (1 << LG) >= n.
//   Static only, like every sparse table. One update = full rebuild.
//   Memory LG * N of your element type. For matrices that gets big fast --
//     consider a segment tree instead, O(log n) per query but O(n) memory.
//   op must be ASSOCIATIVE. It does NOT need to be commutative or idempotent,
//     and it does NOT need an inverse (that is what rules out prefix sums).
//
// SUBPROBLEMS:
//   range sum / xor with O(1) and no inverse    this
//   range product mod p                         this (0s and non-invertible
//                                               residues make prefix products fail)
//   range matrix product                        this, order preserved
//   range composition of functions              this
//   range min/max/gcd                           a PLAIN sparse table is smaller
//                                               and simpler -- use that instead
//   any of the above WITH updates               segment tree, not this
// ============================================================================
const int N = 1 << 18, LG = 18;
ll t[LG][N], a[N];

// any associative operation; commutativity and idempotence not required
ll op(ll x, ll y) { return x + y; }

void build(int n) {
    for (int k = 0; (1 << k) < n; k++) {
        int len = 1 << k;
        for (int mid = len; mid < n; mid += len << 1) {
            t[k][mid - 1] = a[mid - 1];                       // fold leftwards
            for (int i = mid - 2; i >= mid - len; i--)
                t[k][i] = op(a[i], t[k][i + 1]);
            int hi = min(n, mid + len);
            t[k][mid] = a[mid];                               // fold rightwards
            for (int i = mid + 1; i < hi; i++)
                t[k][i] = op(t[k][i - 1], a[i]);
        }
    }
}
// inclusive [l, r], 0 <= l <= r < n
ll query(int l, int r) {
    if (l == r) return a[l];
    int k = __lg(l ^ r);
    return op(t[k][l], t[k][r]);
}
