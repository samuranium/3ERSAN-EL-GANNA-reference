// ============================================================================
// IMPLICIT TREAP -- an array with O(log n) insert/erase ANYWHERE, plus range
//                   reverse, range add, range sum. Expected O(log n) per op.
// ----------------------------------------------------------------------------
// "Implicit" = the key is the POSITION, and position is never stored. It is
// derived from subtree sizes on the way down. That is what lets you insert in
// the middle or cut-and-paste a block -- operations no segment tree can do,
// because a segment tree's indices are fixed forever.
//
// ############################################################################
// #  EVERY DESCENT MUST push() FIRST
// #
// #  split, merge, and any traversal read children. If a pending rev or add is
// #  still sitting on the node, those children are stale and you corrupt the
// #  tree silently -- usually visible only after a reverse overlaps an add.
// #  Rule: push() on entry, pull() on exit. No exceptions.
// #
// #  applyRev() swaps the children IMMEDIATELY and sets the flag for the
// #  subtree below. So rev[x] means "my descendants still owe a reversal",
// #  not "I owe one".
// ############################################################################
//
// EVERYTHING IS BUILT FROM split + merge:
//   split(x, k, a, b)  ->  a gets the first k items, b gets the rest
//   merge(a, b)        ->  concatenation, requires every key in a before b
//
//   range op on [l, r]:   split off [0,l), then [l,r], do the op, merge back.
//   That three-way split is the whole idiom -- see range_add / range_rev.
//
// PITFALLS:
//   Node 0 is the null sentinel and must keep sz = 0, sum = 0. Never touch it.
//   Positions here are 0-INDEXED and ranges are INCLUSIVE [l, r].
//   Priorities must be random. A fixed seed is fine; an INCREASING priority
//     turns the treap into a linked list and every op into O(n).
//   Recursion depth is O(log n) expected -- safe, unlike the tree DFS files.
//   sum overflows int fast; keep ll.
//   Reserve the pool up front (t.reserve(n + q + 5)) so push_back does not
//     reallocate mid-operation.
//
// SUBPROBLEMS:
//   insert / erase at a position          insert_at, erase_at
//   reverse a subarray                    range_rev
//   add v to a subarray                   range_add
//   sum of a subarray                     range_sum
//   CUT AND PASTE a block                 split three ways, merge in a new
//                                         order -- O(log n), no data moved
//   rotate an array by k                  split at k, merge swapped
//   k-th element                          descend on sz, same as split
//   ordered set with duplicates by index  this, or pb_ds for value order
//   persistent version                    copy nodes in split/merge instead of
//                                         mutating -- doubles the memory
//
// NOT THIS: if positions never change, a segment tree is faster and simpler.
//   The treap is for when the SEQUENCE ITSELF is edited.
// ============================================================================
mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

struct Treap {
    struct Node { int pri, sz, l, r; ll val, sum, add; bool rev; };
    vector<Node> t;
    Treap() { t.push_back({0, 0, 0, 0, 0, 0, 0, false}); }   // 0 = null

    int make(ll v) {
        t.push_back({(int)rng(), 1, 0, 0, v, v, 0, false});
        return t.size() - 1;
    }
    void applyAdd(int x, ll v) {
        if (!x) return;
        t[x].val += v, t[x].sum += v * t[x].sz, t[x].add += v;
    }
    void applyRev(int x) {
        if (!x) return;
        t[x].rev ^= 1, swap(t[x].l, t[x].r);
    }
    void push(int x) {
        if (t[x].add) applyAdd(t[x].l, t[x].add), applyAdd(t[x].r, t[x].add), t[x].add = 0;
        if (t[x].rev) applyRev(t[x].l), applyRev(t[x].r), t[x].rev = false;
    }
    void pull(int x) {
        t[x].sz = 1 + t[t[x].l].sz + t[t[x].r].sz;
        t[x].sum = t[x].val + t[t[x].l].sum + t[t[x].r].sum;
    }
    // a gets the first k items of x, b gets the rest
    void split(int x, int k, int &a, int &b) {
        if (!x) return void(a = b = 0);
        push(x);
        if (t[t[x].l].sz >= k) split(t[x].l, k, a, t[x].l), b = x;
        else split(t[x].r, k - t[t[x].l].sz - 1, t[x].r, b), a = x;
        pull(x);
    }
    int merge(int a, int b) {
        if (!a || !b) return a ^ b;
        if (t[a].pri > t[b].pri) { push(a); t[a].r = merge(t[a].r, b); pull(a); return a; }
        push(b); t[b].l = merge(a, t[b].l); pull(b); return b;
    }
    // ---- the three-way split idiom ----
    int insert_at(int root, int pos, ll v) {
        int a, b;
        split(root, pos, a, b);
        return merge(merge(a, make(v)), b);
    }
    int erase_at(int root, int pos) {
        int a, b, c;
        split(root, pos, a, b);
        split(b, 1, b, c);
        return merge(a, c);
    }
    int range_add(int root, int l, int r, ll v) {
        int a, b, c;
        split(root, l, a, b);
        split(b, r - l + 1, b, c);
        applyAdd(b, v);
        return merge(merge(a, b), c);
    }
    int range_rev(int root, int l, int r) {
        int a, b, c;
        split(root, l, a, b);
        split(b, r - l + 1, b, c);
        applyRev(b);
        return merge(merge(a, b), c);
    }
    ll range_sum(int &root, int l, int r) {
        int a, b, c;
        split(root, l, a, b);
        split(b, r - l + 1, b, c);
        ll s = t[b].sum;
        root = merge(merge(a, b), c);
        return s;
    }
    void flatten(int x, vector<ll> &out) {
        if (!x) return;
        push(x);
        flatten(t[x].l, out);
        out.push_back(t[x].val);
        flatten(t[x].r, out);
    }
};
