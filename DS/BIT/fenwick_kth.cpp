// ============================================================================
// FENWICK ORDER STATISTICS -- k-th element by binary lifting, O(log n)
// ----------------------------------------------------------------------------
// Not a binary search over prefix sums (that is O(log^2)). Descend the Fenwick
// itself: try the biggest power of two, and if the prefix there is still short
// of k, take the step and subtract what you consumed. Each bit is one array
// read, so the whole thing is a single O(log n) walk.
//
// ############################################################################
// #  LOG MUST SATISFY  (1 << LOG) >= n
// #
// #  Too SMALL and the walk cannot reach the high indices: it silently returns
// #  a position that is too low. No crash, no assert, just wrong answers on the
// #  tail of the array.
// #  Too LARGE costs a couple of skipped iterations and nothing else -- so when
// #  in doubt, round up. LOG = 20 covers 1e6, LOG = 18 covers 2.6e5.
// #  Exact version: for (int pw = 1 << __lg(n); pw; pw >>= 1)
// ############################################################################
//
// 1-INDEXED. bit[] holds COUNTS (or weights); the array is a multiset over the
// value range [1..n], not a list of elements.
//
// THE MENTAL MODEL: bit[] is a multiset of values. upd(v, +1) inserts one copy
//   of v, upd(v, -1) erases one. kth(k) returns the k-th smallest value. That
//   is an ordered multiset with O(log n) everything, for values in a known
//   small range -- usually faster than pb_ds ordered_set and always available.
//
// PITFALLS:
//   kth(k) with k > total returns n + 1 or garbage past the end -- check
//     pre(n) >= k first if k can exceed the size.
//   kth(0) is meaningless; guard k >= 1.
//   Values must be compressed into [1, n] first if they are large or negative.
//   The `bit[pos + pw] < k` test is STRICT. With <= you get the k-th plus one.
//
// SUBPROBLEMS:
//   k-th smallest in a multiset            kth(k) after upd(v, +1) per element
//   erase / insert a value                 upd(v, -1) / upd(v, +1)
//   k-th SET BIT of a 0/1 array            same thing, weights are 0 and 1
//   k-th free slot / first empty position  invert: store 1 for free, kth(1)
//   "find and remove the k-th"             int v = kth(k); upd(v, -1);
//   rank of a value (how many are <= v)    pre(v)
//   count in a value range [a, b]          pre(b) - pre(a-1)
//   inversions                             sweep left to right, add
//                                          (i - pre(a[i])) then upd(a[i], +1)
//   sliding-window median                  two of these, or one plus kth(k/2)
//   Josephus / process every k-th          kth + erase in a loop, O(n log n)
// ============================================================================
const int N = 1 << 20, LOG = 20;   // (1 << LOG) >= n -- see the box
ll bit[N + 1];
int n;                              // value range is [1, n]; set before use

void upd(int i, ll v) {
    for (; i <= n; i += i & -i) bit[i] += v;
}
ll pre(int i) {                     // sum of counts over values [1, i]
    ll s = 0;
    for (; i > 0; i -= i & -i) s += bit[i];
    return s;
}
// smallest index whose prefix sum is >= k. Assumes 1 <= k <= pre(n).
int kth(ll k) {
    int pos = 0;
    for (int pw = 1 << LOG; pw; pw >>= 1)
        if (pos + pw <= n && bit[pos + pw] < k) pos += pw, k -= bit[pos];
    return pos + 1;
}
