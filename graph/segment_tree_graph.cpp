// ============================================================================
// SEGMENT TREE GRAPH -- "edge from u to EVERY node in [l,r]" in O(log n) edges
// ----------------------------------------------------------------------------
// The problem: a statement gives you n^2 edges implicitly ("from portal i you
// may fly to any city in [l,r] for cost w"). Building them is 1e10 edges.
//
// Build TWO segment trees over the vertices, both with 2n internal nodes:
//
//   OUT tree   parent -> child, weight 0.  Entering an OUT node lets you exit
//              at any leaf below it. Use it as the TARGET of a range edge.
//   IN  tree   child -> parent, weight 0.  Entering any leaf below an IN node
//              reaches it. Use it as the SOURCE of a range edge.
//
// Then:
//     u -> [l,r] with cost w    add u -> (O(log n) OUT nodes covering [l,r]), w
//     [l,r] -> v with cost w    add (O(log n) IN nodes covering [l,r]) -> v, w
//
// Total O(n + q log n) nodes and edges. Run plain Dijkstra/BFS on the result.
// Nothing about the shortest path algorithm changes -- only the graph.
//
// ############################################################################
// #  THE TWO TREES SHARE THEIR LEAVES, AND THAT IS MANDATORY
// #
// #  Leaf i of the OUT tree and leaf i of the IN tree must be the SAME vertex
// #  id -- the real vertex i. Otherwise arriving via a range edge leaves you
// #  stranded in a tree that has no way back down to the real graph.
// #
// #  Below, the real vertices are 0..n-1 and both trees allocate their
// #  INTERNAL nodes above n, with the leaf slot mapped straight to i.
// ############################################################################
//
// ############################################################################
// #  THE ZERO-WEIGHT SKELETON EDGES GO ONE WAY ONLY
// #
// #  OUT: parent -> child.  IN: child -> parent.  Adding both directions in
// #  either tree creates free travel between arbitrary vertices, and every
// #  distance collapses toward 0. The symptom is "all my answers are too
// #  small", and it survives every small test where the answer happens to be
// #  reachable directly.
// ############################################################################
//
// PITFALLS:
//   Size everything for n + 4n + 4n vertices and (q * 2 log n + 4n) edges.
//     Under-reserving a vector<vector<>> is fine; under-reserving a flat edge
//     array is not.
//   dist[] is indexed by SEGMENT TREE node, not by real vertex. Read the
//     answer at dist[v] for v < n only.
//   A single-vertex range still works; do not special-case it.
//   If all costs are equal use BFS; if costs are 0/w use 0-1 BFS -- the graph
//     construction is identical.
//   The same construction handles "[l1,r1] -> [l2,r2]": make one helper node
//     per query, IN-range -> helper -> OUT-range. Do NOT connect the ranges
//     directly, that is O(log^2 n) edges per query for no reason.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   "from u you may go to any node in [l,r]"        u -> OUT range
//   "from any node in [l,r] you may go to v"        IN range -> v
//   "from any node in [l1,r1] to any in [l2,r2]"    IN range -> helper ->
//                                                   OUT range
//   teleports / portals / flight ranges             this, verbatim
//   "connect all nodes of a colour at cost w"       simpler: one virtual node
//                                                   per colour, in-edge 0 and
//                                                   out-edge w. No tree needed
//   "connect all nodes in a row / column"           same virtual-node trick
//   2-SAT with "at least one of x_l..x_r is true"   the same two trees, over
//                                                   the implication graph
//   range edges on VALUES rather than indices       sort, then it is a range
//                                                   of indices
//   MST with implicit range edges                   this does not help --
//                                                   Boruvka + a range structure
//
// NOT THIS:
//   the ranges are all the same range        -> one virtual node, done.
//   only a handful of range edges (< ~100)   -> expanding them is simpler.
//   you need PATH COUNTS, not distances      -> the skeleton's zero edges make
//     the path count meaningless; each real path has many tree encodings.
// ============================================================================
const ll INF = 1e18;

int n;                                   // real vertices are 0 .. n-1
int cnt;                                 // next free segment-tree node id
vector<pair<int, ll>> g[1200005];        // adjacency: (to, weight)
int outn[800005], inn[800005];           // node ids for each segment tree node

// build over [lo,hi]; `v` is the recursion index into outn/inn
void build(int v, int lo, int hi) {
    if (lo == hi) { outn[v] = inn[v] = lo; return; }   // SHARED leaf = vertex
    outn[v] = cnt++, inn[v] = cnt++;
    int mid = lo + hi >> 1;
    build(2 * v, lo, mid);
    build(2 * v + 1, mid + 1, hi);
    g[outn[v]].push_back({outn[2 * v], 0});            // OUT: parent -> child
    g[outn[v]].push_back({outn[2 * v + 1], 0});
    g[inn[2 * v]].push_back({inn[v], 0});              // IN:  child -> parent
    g[inn[2 * v + 1]].push_back({inn[v], 0});
}
void init(int n_) {
    n = n_, cnt = n;
    build(1, 0, n - 1);
}
// from `src` to every vertex in [l,r], cost w
void add_to_range(int v, int lo, int hi, int l, int r, int src, ll w) {
    if (r < lo || hi < l) return;
    if (l <= lo && hi <= r) { g[src].push_back({outn[v], w}); return; }
    int mid = lo + hi >> 1;
    add_to_range(2 * v, lo, mid, l, r, src, w);
    add_to_range(2 * v + 1, mid + 1, hi, l, r, src, w);
}
// from every vertex in [l,r] to `dst`, cost w
void add_from_range(int v, int lo, int hi, int l, int r, int dst, ll w) {
    if (r < lo || hi < l) return;
    if (l <= lo && hi <= r) { g[inn[v]].push_back({dst, w}); return; }
    int mid = lo + hi >> 1;
    add_from_range(2 * v, lo, mid, l, r, dst, w);
    add_from_range(2 * v + 1, mid + 1, hi, l, r, dst, w);
}
// convenience wrappers
void edge_to_range(int u, int l, int r, ll w)   { add_to_range(1, 0, n - 1, l, r, u, w); }
void edge_from_range(int l, int r, int v, ll w) { add_from_range(1, 0, n - 1, l, r, v, w); }
// [l1,r1] -> [l2,r2] : one helper node in between, O(log n) edges each side
void edge_range_range(int l1, int r1, int l2, int r2, ll w) {
    int helper = cnt++;
    add_from_range(1, 0, n - 1, l1, r1, helper, 0);
    add_to_range(1, 0, n - 1, l2, r2, helper, w);
}

vector<ll> dijkstra(int s) {
    vector<ll> d(cnt, INF);
    priority_queue<pair<ll, int>, vector<pair<ll, int>>, greater<>> pq;
    d[s] = 0, pq.push({0, s});
    while (!pq.empty()) {
        auto [cur, u] = pq.top(); pq.pop();
        if (cur > d[u]) continue;                      // keep the stale check
        for (auto [v, w] : g[u])
            if (d[u] + w < d[v]) d[v] = d[u] + w, pq.push({d[v], v});
    }
    d.resize(n);                                       // real vertices only
    return d;
}
