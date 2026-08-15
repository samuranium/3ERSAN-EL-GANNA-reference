// ============================================================================
// KRUSKAL RECONSTRUCTION TREE -- turns "bottleneck" into "ancestor"
// ----------------------------------------------------------------------------
// Run Kruskal. Every time two components merge, create a NEW internal node
// whose value is that edge's weight and whose two children are the roots of
// the merged components. The result is a binary tree with:
//
//     n leaves  = the original vertices
//     n-1 internal nodes, values NON-DECREASING towards the root
//
// and one identity that solves a whole family of problems:
//
//     min possible MAXIMUM edge on any u->v path  =  val[ lca(u, v) ]
//
// The consequence that matters more: "every vertex reachable from u using only
// edges of weight <= w" is exactly the LEAF SET OF A SUBTREE -- walk up from u
// while the parent's value is <= w, then take that subtree. A threshold query
// becomes a subtree query, so Euler tour + any range structure answers it.
//
// ############################################################################
// #  IT IS THE *MAXIMUM* THAT BECOMES THE LCA, AND ONLY ON THE MST
// #
// #  val[lca(u,v)] is the minimax edge -- "minimise the largest edge you must
// #  cross". For the MAXIMIN version ("maximise the smallest edge", widest
// #  path) build the tree from the MAXIMUM spanning tree: sort DESCENDING.
// #  Same code, one comparator flipped. Using the wrong direction gives an
// #  answer with the right shape and the wrong value.
// ############################################################################
//
// ############################################################################
// #  ALLOCATE 2n NODES AND KEEP THE DSU SEPARATE FROM THE TREE
// #
// #  The DSU tracks which KRT node currently represents each component; it is
// #  not the tree itself. `f[find(u)]` is the current root of u's component.
// #  Reusing the DSU parent array as the tree parent array loses the
// #  structure the moment path compression runs.
// #
// #  A disconnected graph yields a FOREST -- several roots. Handle it, or
// #  lca() walks off the top. val[] of a missing ancestor is +INF.
// ############################################################################
//
// PITFALLS:
//   Leaves have value 0 (or -INF for the maximin version), not the edge value.
//   Equal weights: any order is fine, the minimax answer is unique.
//   The tree has 2n-1 nodes -- size every array 2n, including the LCA table.
//   Answering "reachable using weight <= w" needs the ancestor jump, which is
//     binary lifting on VALUE, not on depth. Same table, different predicate.
//   Building this on a dense graph is O(m log m) -- the sort dominates. On a
//     complete graph with formula weights, use Boruvka instead.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   minimise the maximum edge on a path u->v        val[lca(u,v)]
//   maximise the minimum edge (widest path)         same, built from the MAX
//                                                   spanning tree
//   smallest w such that u and v are connected      val[lca(u,v)] -- identical
//                                                   question
//   "everything reachable from u with weight <= w"  jump up while val <= w,
//                                                   take that subtree
//   k-th largest / count of such reachable nodes    Euler tour that subtree +
//                                                   a merge sort tree or a
//                                                   persistent segment tree
//   offline "edges appear in weight order"          process queries sorted by
//                                                   w; equivalently this tree
//   second-best MST                                 different tool --
//                                                   second_best_mst.cpp
//   is edge e in SOME mst / EVERY mst               some: its endpoints are in
//                                                   different components
//                                                   before its weight class;
//                                                   every: it is a bridge
//                                                   within that class
//   "min time until u and v can meet"               weights are times, then
//                                                   val[lca(u,v)]
//
// NOT THIS:
//   plain shortest path (sum of weights) -> Dijkstra. This tree knows nothing
//     about sums.
//   the graph changes -> rebuild; there is no incremental version.
//   you only need ONE minimax query -> binary search + DSU is shorter.
// ============================================================================
// needs: DSU (find/unite by size). graph/dsu/ has one; this file keeps its own
// so it can stand alone.
struct Edge { int u, v; ll w; };

int n, m;                      // original vertices 0..n-1
Edge es[400005];

int tot;                       // number of KRT nodes built (up to 2n-1)
int kl[400005], kr[400005], kpar[400005];
ll  kval[400005];
int dsu[400005], rep[400005];  // dsu over components; rep[root] = its KRT node

int find(int x) { return dsu[x] == x ? x : dsu[x] = find(dsu[x]); }

// build the MIN version. Sort descending instead for the maximin/widest-path
// version -- nothing else changes.
void build_krt() {
    sort(es, es + m, [](const Edge &a, const Edge &b) { return a.w < b.w; });
    tot = n;
    for (int i = 0; i < n; i++) dsu[i] = i, rep[i] = i, kval[i] = 0,
                                kl[i] = kr[i] = -1;
    for (int i = 0; i < 2 * n; i++) kpar[i] = -1;
    for (int i = 0; i < m; i++) {
        int a = find(es[i].u), b = find(es[i].v);
        if (a == b) continue;
        int node = tot++;
        kl[node] = rep[a], kr[node] = rep[b];
        kval[node] = es[i].w;
        kpar[rep[a]] = kpar[rep[b]] = node;
        dsu[a] = b;                              // merge the components
        rep[b] = node;                           // b's root now maps here
    }
}

// ---- binary lifting over the KRT, for lca() and the value jump ------------
const int LOG = 20;
int up[LOG][400005], dep[400005];

void krt_lca_init() {
    // roots first: a forest is fine
    for (int v = tot - 1; v >= 0; v--) {
        if (kpar[v] < 0) dep[v] = 0, up[0][v] = v;
        else dep[v] = dep[kpar[v]] + 1, up[0][v] = kpar[v];
    }
    for (int k = 1; k < LOG; k++)
        for (int v = 0; v < tot; v++) up[k][v] = up[k - 1][up[k - 1][v]];
}
int lca(int u, int v) {
    if (dep[u] < dep[v]) swap(u, v);
    int d = dep[u] - dep[v];
    for (int k = 0; k < LOG; k++) if (d >> k & 1) u = up[k][u];
    if (u == v) return u;
    for (int k = LOG - 1; k >= 0; k--)
        if (up[k][u] != up[k][v]) u = up[k][u], v = up[k][v];
    return kpar[u];
}
// minimise the maximum edge on any path u->v; -1 if they are not connected
ll minimax(int u, int v) {
    int a = u, b = v;
    while (kpar[a] >= 0) a = kpar[a];
    while (kpar[b] >= 0) b = kpar[b];
    if (a != b) return -1;                       // different components
    return kval[lca(u, v)];
}
// highest ancestor of v whose value is <= w. Its leaves are exactly the set
// reachable from v using edges of weight <= w.
int reach_root(int v, ll w) {
    for (int k = LOG - 1; k >= 0; k--) {
        int a = up[k][v];
        if (a != v && kval[a] <= w) v = a;
    }
    return v;
}
