// ============================================================================
// BORUVKA + XOR-MST -- MST when the edges are a FORMULA, not a list
// ----------------------------------------------------------------------------
// Kruskal and Prim both need the edge list. When the graph is complete and the
// weight is given by a rule (xor, |a_i - a_j|, Manhattan distance), the list is
// n^2 edges and cannot be built.
//
// Boruvka never enumerates edges. Each ROUND, every current component finds its
// own cheapest outgoing edge and merges along it. Every round at least halves
// the component count, so there are O(log n) rounds. All you need is:
//
//     "given a component, find its cheapest edge to ANY vertex outside it"
//
// and that is a data-structure question, not a graph one.
//
// ############################################################################
// #  TIE-BREAKING IS MANDATORY OR BORUVKA BUILDS A CYCLE
// #
// #  If two components each pick the same-weight edge toward each other, both
// #  merges are recorded and you get a duplicate; with three components in a
// #  cycle of equal weights you get an actual cycle and the "tree" has n
// #  edges.
// #
// #  Fix: break ties by EDGE INDEX (or by the pair (weight, index)) so the
// #  choice is a strict total order. Then "u picks e" and "v picks e" is the
// #  same edge and the DSU merge is idempotent. Never compare on weight alone.
// ############################################################################
//
// ############################################################################
// #  XOR-MST: THE TRIE *IS* THE RECURSION TREE
// #
// #  Sort the values and build a binary trie. At the highest bit where the
// #  set splits, EVERY spanning tree must cross that split exactly once, and
// #  the cheapest crossing edge is min over x in the 0-side of (x xor best
// #  match in the 1-side) -- one trie descent per element of the smaller
// #  side.
// #
// #  Recursing on both halves and adding that one crossing edge gives the
// #  whole MST in O(n log^2 C). Recurse on the SMALLER side's queries or the
// #  bound degrades to O(n^2).
// ############################################################################
//
// PITFALLS:
//   Boruvka: recompute find() for both endpoints when applying a chosen edge --
//     an earlier merge in the same round may have already joined them.
//   A round that finds no edge at all means the graph is disconnected; stop.
//   Best-edge array must be reset every round.
//   XOR-MST: duplicate values contribute a 0-weight edge each; dedupe first
//     and add (count - 1) zeros, or the trie recursion sees an empty side.
//   XOR-MST recursion depth is the bit count (30 or 60), not n.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   MST of a complete graph, weight = a_i xor a_j       xor_mst()
//   MST with weight = |a_i - a_j|                       sort; only adjacent
//                                                       pairs can be MST edges
//   MST with Manhattan distance                         8 octant sweeps to get
//                                                       O(n log n) candidate
//                                                       edges, then Kruskal
//   MST where the weight is any formula                 Boruvka + a structure
//                                                       that answers "cheapest
//                                                       edge leaving this set"
//   MST of a normal edge list                           Kruskal or Prim --
//                                                       graph/MST/. Boruvka is
//                                                       not the tool
//   parallel / distributed MST                          Boruvka, it is the
//                                                       naturally parallel one
//   "minimum total xor to connect all values"           xor_mst()
//   second-best MST                                     graph/MST/second_best_mst.cpp
//   bottleneck / threshold queries on the MST           graph/MST/kruskal_tree.cpp
//
// NOT THIS:
//   you already have the edge list and m is small -> Kruskal. Shorter, and
//     the sort dominates either way.
//   the "graph" is a tree already                 -> nothing to do.
// ============================================================================

// ---------------------------------------------------------------------------
// [A] BORUVKA on an explicit edge list -- the template to adapt. Replace the
//     "find the cheapest outgoing edge per component" loop with whatever
//     structure the implicit graph needs.
// ---------------------------------------------------------------------------
int par[200005], sz_[200005];
int find(int x) { return par[x] == x ? x : par[x] = find(par[x]); }
bool unite(int a, int b) {
    a = find(a), b = find(b);
    if (a == b) return false;
    if (sz_[a] < sz_[b]) swap(a, b);
    par[b] = a, sz_[a] += sz_[b];
    return true;
}

struct E { int u, v; ll w; };

ll boruvka(int n, vector<E> &es) {
    for (int i = 0; i < n; i++) par[i] = i, sz_[i] = 1;
    ll total = 0;
    int comps = n;
    vector<int> best(n);
    while (comps > 1) {
        fill(best.begin(), best.end(), -1);
        for (int i = 0; i < (int)es.size(); i++) {
            int a = find(es[i].u), b = find(es[i].v);
            if (a == b) continue;
            // strict total order: (weight, index). Weight alone builds cycles.
            auto better = [&](int i, int j) {
                if (j < 0) return true;
                if (es[i].w != es[j].w) return es[i].w < es[j].w;
                return i < j;
            };
            if (better(i, best[a])) best[a] = i;
            if (better(i, best[b])) best[b] = i;
        }
        bool any = false;
        for (int v = 0; v < n; v++) {
            if (best[v] < 0) continue;
            E &e = es[best[v]];
            if (unite(e.u, e.v)) total += e.w, comps--, any = true;
        }
        if (!any) break;                       // disconnected
    }
    return comps == 1 ? total : -1;            // -1 = not connected
}

////////////////////////////////////////////////////////////////////////////////
// [B] XOR-MST -- O(n log^2 C) with no trie object at all.
//
// Sort the values once. At bit b, the array splits into a contiguous block
// with bit b = 0 and one with bit b = 1 (because it is sorted). Recurse into
// both, then pay ONE crossing edge: the minimum xor between the two blocks,
// found by walking each element of one side down the other side's range.
//
// The "trie" is exactly this sorted-range recursion, which is why no nodes are
// needed.
////////////////////////////////////////////////////////////////////////////////
vector<ll> a_;                                  // sorted, deduped

// min (x xor y) for x in [l1,r1), y in [l2,r2), both sorted ranges, at bit b
ll min_cross(int l1, int r1, int l2, int r2, int b) {
    ll best = LLONG_MAX;
    for (int i = l1; i < r1; i++) {
        ll x = a_[i], cur = 0;
        int lo = l2, hi = r2;
        for (int k = b; k >= 0; k--) {
            int d = x >> k & 1;
            // the sorted block [lo,hi) splits at bit k
            int mid = lo;
            while (mid < hi && !(a_[mid] >> k & 1)) mid++;
            int wl = d ? mid : lo, wr = d ? hi : mid;      // prefer same bit
            if (wl < wr) { lo = wl, hi = wr; }
            else { cur |= 1LL << k;                        // forced to differ
                   if (d) lo = lo, hi = mid; else lo = mid; }
        }
        best = min(best, cur);
    }
    return best;
}

ll xor_mst(int l, int r, int b) {               // values a_[l..r), bit b
    if (r - l <= 1 || b < 0) return 0;
    int mid = l;
    while (mid < r && !(a_[mid] >> b & 1)) mid++;
    if (mid == l || mid == r) return xor_mst(l, r, b - 1);   // no split here
    ll left = xor_mst(l, mid, b - 1), right = xor_mst(mid, r, b - 1);
    // recurse the queries from the SMALLER side to keep the log^2 bound
    ll cross = (mid - l <= r - mid) ? min_cross(l, mid, mid, r, b)
                                    : min_cross(mid, r, l, mid, b);
    return left + right + cross;
}

// full entry point: handles duplicates (each extra copy is a 0-weight edge)
ll xor_mst_all(vector<ll> v) {
    sort(v.begin(), v.end());
    v.erase(unique(v.begin(), v.end()), v.end());   // duplicates cost 0
    a_ = v;
    if (a_.size() <= 1) return 0;
    int B = 0;
    while ((1LL << B) <= a_.back()) B++;
    return xor_mst(0, a_.size(), B);
}
