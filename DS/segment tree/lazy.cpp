// ============================================================================
// LAZY SEGMENT TREE -- range update, range query, O(log n)
// ----------------------------------------------------------------------------
// ############################################################################
// #  merge AND apply MUST AGREE ON THE SAME OPERATION
// #
// #  As configured below:  merge = MAX,  apply does NOT scale by len.
// #  For SUM you must change BOTH:
// #      merge   return {x.f + y.f};
// #      apply   x.f += z.add * len;
// #
// #  Mixing them -- a max merge with a *len apply -- is the classic bug. Every
// #  range-add inflates the maxima by the segment length, and it still looks
// #  right on a 3-element test.
// #  len is passed in for exactly this reason: SUM needs it, MAX/MIN ignore it.
// ############################################################################
//
// [L, R] inclusive, 1-indexed. Build over a[1..n] with build(1, 1, n).
//
// PITFALLS:
//   Lazy::none() decides whether prop() can early-exit. For range-ASSIGN the
//     lazy has to carry a "has value" flag -- 0 is a legal assignment, so
//     !add is the wrong test there. See VARIANTS.
//   prop() must run before recursing in BOTH update and query.
//   compose() applies z ON TOP of an existing x. For add they commute; for
//     assign they do not -- z wins outright.
//   query() returns a Node, not a scalar, so custom merges compose properly.
//
// ----------------------------------------------------------------------------
// VARIANTS -- only merge/apply/compose/none change, never the traversal
// ----------------------------------------------------------------------------
//   range add + range sum      merge x.f+y.f      apply f += add*len
//   range add + range max      merge max          apply f += add          <- now
//   range add + range min      merge min          apply f += add
//   range assign + range sum   lazy {val, has}    apply f = val*len
//   range assign + range max   lazy {val, has}    apply f = val
//   range gcd (add on a diff array)               see SUBPROBLEMS [3]
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
// [1] FIRST INDEX >= i WITH VALUE >= x  -- descend() below. O(log n), not
//     O(log^2): the tree[u].f < x test prunes whole subtrees, so the walk only
//     ever enters a node that actually contains an answer.
//     Mirror it for "first <= x" with a min tree.
//     Classic use: hotel-room / bin-packing assignment ("first shelf with
//     enough space"), and next-greater-element with updates.
//
// [2] MAX SUBARRAY SUM as a custom merge. Node carries {sum, pre, suf, best}:
//         sum  = x.sum + y.sum
//         pre  = max(x.pre, x.sum + y.pre)
//         suf  = max(y.suf, y.sum + x.suf)
//         best = max(max(x.best, y.best), x.suf + y.pre)
//     Identity is {0, -INF, -INF, -INF}. DS/segment tree/segment tree.cpp
//     already uses this shape for increasing runs -- same idea, different
//     payload.
//
// [3] RANGE GCD WITH RANGE ADD. gcd is not addable, so build the tree over the
//     DIFFERENCE array d[i] = a[i] - a[i-1]. A range add to [l,r] touches only
//     d[l] and d[r+1] -- two POINT updates. Then
//         gcd(a[l..r]) = gcd(a[l], gcd(d[l+1..r]))
//     keeping a[l] in a separate BIT. Same trick works for range-gcd queries
//     with range-add updates generally.
//
// [4] COUNT / POSITION OF THE MAX in a range: extend Node to {f, cnt} and
//     merge by comparing f, summing cnt on ties. Costs nothing extra.
//
// [5] IF YOU NEED "range assign + range add together" the lazy becomes a pair
//     (assign then add) and compose gets order-sensitive. Doable, but check
//     whether a simpler model exists first.
// ============================================================================
const int N = 200'123;
ll a[N];

struct SegmentTree {
    struct Node { ll f; };
    struct Lazy {
        ll add;
        bool none() const { return !add; }
    };
    Node tree[N << 2];
    Lazy lazy[N << 2];

    Node single(ll x) { return {x}; }
    Node merge(const Node &x, const Node &y) { return {max(x.f, y.f)}; }
    void apply(Node &x, const Lazy &z, int len) { x.f += z.add; }  // MAX: no *len
    void compose(Lazy &x, const Lazy &z) { x.add += z.add; }

    void build(int u, int st, int en) {
        lazy[u] = {};
        if (st == en) return void(tree[u] = single(a[st]));
        int mid = st + en >> 1;
        build(u << 1, st, mid);
        build(u << 1 | 1, mid + 1, en);
        tree[u] = merge(tree[u << 1], tree[u << 1 | 1]);
    }
    void prop(int u, int st, int mid, int en) {
        if (lazy[u].none()) return;
        apply(tree[u << 1], lazy[u], mid - st + 1), compose(lazy[u << 1], lazy[u]);
        apply(tree[u << 1 | 1], lazy[u], en - mid), compose(lazy[u << 1 | 1], lazy[u]);
        lazy[u] = {};
    }
    void update(int u, int st, int en, int i, int j, const Lazy &v) {
        if (i <= st && en <= j) return apply(tree[u], v, en - st + 1), compose(lazy[u], v);
        int mid = st + en >> 1;
        prop(u, st, mid, en);
        if (i <= mid) update(u << 1, st, mid, i, j, v);
        if (j > mid) update(u << 1 | 1, mid + 1, en, i, j, v);
        tree[u] = merge(tree[u << 1], tree[u << 1 | 1]);
    }
    Node query(int u, int st, int en, int i, int j) {
        if (i <= st && en <= j) return tree[u];
        int mid = st + en >> 1;
        prop(u, st, mid, en);
        if (j <= mid) return query(u << 1, st, mid, i, j);
        if (i > mid) return query(u << 1 | 1, mid + 1, en, i, j);
        return merge(query(u << 1, st, mid, i, j), query(u << 1 | 1, mid + 1, en, i, j));
    }
    // [1] first index >= i whose value is >= x, or -1. Needs a MAX merge.
    int descend(int u, int st, int en, int i, ll x) {
        if (en < i || tree[u].f < x) return -1;
        if (st == en) return st;
        int mid = st + en >> 1;
        prop(u, st, mid, en);
        int r = descend(u << 1, st, mid, i, x);
        return r != -1 ? r : descend(u << 1 | 1, mid + 1, en, i, x);
    }
} t;
