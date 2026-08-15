// ============================================================================
// LI CHAO TREE -- min over a set of lines at a query point, O(log C) per op
// ----------------------------------------------------------------------------
// Each node owns ONE line: the one that wins at that node's midpoint. Insert
// walks down deciding, at every node, which of the two lines to keep and which
// to push into the half where it might still win. Two lines cross at most once,
// so exactly one half can still change hands -- that is why insert is a single
// root-to-leaf walk and not a rebuild.
//
// ############################################################################
// #  WHEN TO USE THIS INSTEAD OF CHT (DP/optimization/CHT.cpp)
// #
// #  CHT / LineContainer wins normally: O(1) memory per line, no coordinate
// #  domain, smaller constant. Reach for LI CHAO only when one of these is
// #  true --
// #
// #    1. A LINE IS ONLY VALID ON A RANGE of x. insert_seg() below. CHT cannot
// #       express this at all. This is the main reason the file exists.
// #    2. You need it PERSISTENT or MERGEABLE (small-to-large over subtrees).
// #       One line per node makes both easy; a hull does not.
// #    3. The slope/query order is adversarial AND you do not trust the hull's
// #       floor-division edge cases.
// #
// #  Cost of choosing it: you must bound the x domain up front, by compressing
// #  the query points or fixing [LO, HI].
// ############################################################################
//
// ############################################################################
// #  THIS FILE IS THE MINIMUM. FOR MAXIMUM, NEGATE.
// #      insert({-m, -c});     answer = -query(x);
// #  Flipping the comparisons instead works but you must also flip the
// #  sentinel, and half-flipping it is the standard way to get a silent wrong
// #  answer here.
// ############################################################################
//
// PITFALLS:
//   m * x overflows before the comparison does. |m| * |x| must fit in ll --
//     with m and x both up to 1e9 that is 1e18, which fits, but 1e18 + c does
//     not always. Keep the sentinel well below LLONG_MAX (INF/4 below).
//   The domain [LO, HI] is INCLUSIVE and fixed at build time. Query outside it
//     reads a leaf that was never meant for you -- clamp or compress first.
//   Node count is 4 * (HI - LO + 1) for the array version. At 1e6 that is
//     4e6 Lines = 64 MB. Compress the x values if you can; the tree only ever
//     needs the points you will actually query.
//   Lines may be inserted in ANY order and queries interleaved freely. That is
//     the whole point -- no monotonicity requirement anywhere.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   dp[i] = min_j (m_j * x_i + c_j)        the CHT family; either tool works
//   ... with arbitrary slope order          this, or LineContainer
//   ... where line j only applies for       ONLY this -- insert_seg()
//       x in [l_j, r_j]
//   min cost with k segments               pair with D&C opt or aliens trick
//   convex hull trick on a TREE            Li Chao merged small-to-large, or
//                                          insert on entry / rollback on exit
//   "buy machine j, then produce x units"  classic: m = rate, c = fixed cost
//   min over parabolas / non-lines         Li Chao works for ANY family where
//                                          two members cross at most once in
//                                          the domain -- state that property
//                                          before assuming it
//
// NOT THIS: slopes AND queries both monotone -> a plain deque hull is O(1)
//   amortised and ten lines. Only reach here when that fails.
// ============================================================================
const ll LINF = (ll)4e18 / 4;

struct Line {
    ll m, c;
    ll operator()(ll x) const { return m * x + c; }
};
const Line NOLINE = {0, LINF};        // evaluates to +inf everywhere

const int LO = 0, HI = 1 << 20;       // inclusive x domain
Line tr[4 * (1 << 20)];

void lc_init(int u, int l, int r) {
    tr[u] = NOLINE;
    if (l == r) return;
    int mid = (l + r) >> 1;
    lc_init(u << 1, l, mid), lc_init(u << 1 | 1, mid + 1, r);
}
// f applies to the WHOLE domain
void insert(int u, int l, int r, Line f) {
    while (true) {
        int mid = (l + r) >> 1;
        bool lef = f(l) < tr[u](l);
        bool md = f(mid) < tr[u](mid);
        if (md) swap(tr[u], f);       // f wins the midpoint: keep it here
        if (l == r) return;
        if (lef != md) u = u << 1, r = mid;          // they cross on the left
        else u = u << 1 | 1, l = mid + 1;            // ... or on the right
    }
}
// f applies ONLY on [ql, qr]. This is what CHT cannot do.
void insert_seg(int u, int l, int r, int ql, int qr, Line f) {
    if (qr < l || r < ql) return;
    if (ql <= l && r <= qr) return insert(u, l, r, f);
    int mid = (l + r) >> 1;
    insert_seg(u << 1, l, mid, ql, qr, f);
    insert_seg(u << 1 | 1, mid + 1, r, ql, qr, f);
}
// minimum over every line covering x
ll query(int u, int l, int r, int x) {
    ll res = tr[u](x);
    while (l != r) {
        int mid = (l + r) >> 1;
        if (x <= mid) u = u << 1, r = mid;
        else u = u << 1 | 1, l = mid + 1;
        res = min(res, tr[u](x));
    }
    return res;
}
