// ============================================================================
// STEINER TREE IN GRAPHS -- cheapest tree connecting k terminals, k <= ~12
// ----------------------------------------------------------------------------
// "Pick the minimum-weight set of edges connecting these k special vertices;
// you may route through any other vertices." NP-hard in k, polynomial in n:
//
//     d[S][v] = min weight of a tree that contains vertex v and every
//               terminal in the mask S
//
// Two transitions, applied per mask in increasing order:
//     MERGE    d[S][v] = min over submasks T of  d[T][v] + d[S^T][v]
//              two trees meeting at the same vertex v
//     GROW     d[S][v] = min over edges (u,v,w) of  d[S][u] + w
//              extend the tree by one edge -- this is a shortest-path relax,
//              so run DIJKSTRA over the whole d[S][*] layer
//
// O(3^k * n + 2^k * (m + n log n)). The answer is min over v of d[full][v].
//
// ############################################################################
// #  THE GROW STEP IS A DIJKSTRA, NOT A SINGLE PASS
// #
// #  After the merge step, d[S][*] is a valid set of starting distances but
// #  NOT final: growing through vertex u can improve v, which can then
// #  improve w. One relaxation sweep over the edges leaves those chains
// #  unresolved, and the answer comes out too large on any test where the
// #  cheapest connection routes through two or more non-terminal vertices.
// #
// #  Seed the priority queue with every (d[S][v], v) that is finite, then run
// #  Dijkstra normally.
// ############################################################################
//
// ############################################################################
// #  MERGE BEFORE GROW, AND ONLY OVER PROPER SUBMASKS
// #
// #  For each S: do all the merges first, THEN the Dijkstra. Swapping them
// #  loses the trees that must be joined before being extended.
// #
// #  The submask loop must skip T = 0 and T = S -- `d[0][v]` is 0 and merging
// #  with it is a no-op that costs nothing but time, while T = S makes
// #  d[S][v] depend on itself.
// ############################################################################
//
// PITFALLS:
//   d[1<<i][terminal_i] = 0 is the base case. Every other d[S][v] starts INF.
//   INF must survive one addition: 1e18/4, not LLONG_MAX.
//   The result is a TREE weight, not a path length -- do not compare it to a
//     shortest path.
//   k <= 12 is comfortable (3^12 * n). k = 15 needs n small.
//   Terminals may repeat or coincide with each other; dedupe first.
//   Disconnected graph: the answer stays INF, which means impossible.
//   Reconstructing the actual edge set needs a parent record per (S,v) --
//     store which transition won.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   connect k special vertices, minimum total cost   this
//   "minimum network joining these cities"           this
//   connect k terminals on a TREE                    trivial: virtual tree,
//                                                    sum of its edges --
//                                                    DS/LCA/virtual_tree.cpp
//   k = 2                                            plain Dijkstra
//   k = n (all vertices)                             minimum spanning tree
//   "each terminal must reach ANY of a group"        add a virtual terminal
//                                                    with 0-edges to the group
//   partition terminals into groups, each connected  d[full][v] over the
//                                                    partition -- a second
//                                                    submask DP on top
//   Steiner FOREST (pairs, not one tree)             different problem; the
//                                                    submask DP does not
//                                                    directly apply
//   vertex weights instead of edge weights           push the weight onto every
//                                                    incoming edge, add the
//                                                    root's own weight at the end
//
// NOT THIS:
//   the graph is a TREE  -> virtual tree, O(k log k). Never this DP.
//   k is large (> ~15)   -> no exact algorithm; the problem wants something else.
//   you need to connect ALL vertices -> that is just an MST.
// ============================================================================
const ll INF = 1e18 / 4;

int n, k;                             // n vertices (0-indexed), k terminals
vector<pair<int, ll>> g[3005];        // (to, weight)
int term[16];
ll d[1 << 12][3005];

ll steiner() {
    int full = (1 << k) - 1;
    for (int S = 0; S <= full; S++)
        for (int v = 0; v < n; v++) d[S][v] = INF;
    for (int i = 0; i < k; i++) d[1 << i][term[i]] = 0;

    for (int S = 1; S <= full; S++) {
        // ---- MERGE: two trees rooted at the same v ------------------------
        for (int v = 0; v < n; v++)
            for (int T = (S - 1) & S; T; T = (T - 1) & S) {   // proper submasks
                if (d[T][v] >= INF || d[S ^ T][v] >= INF) continue;
                d[S][v] = min(d[S][v], d[T][v] + d[S ^ T][v]);
            }
        // ---- GROW: a full Dijkstra over this layer ------------------------
        priority_queue<pair<ll, int>, vector<pair<ll, int>>, greater<>> pq;
        for (int v = 0; v < n; v++) if (d[S][v] < INF) pq.push({d[S][v], v});
        while (!pq.empty()) {
            auto [cur, u] = pq.top(); pq.pop();
            if (cur > d[S][u]) continue;                      // stale check
            for (auto [v, w] : g[u])
                if (d[S][u] + w < d[S][v])
                    d[S][v] = d[S][u] + w, pq.push({d[S][v], v});
        }
    }
    ll best = INF;
    for (int v = 0; v < n; v++) best = min(best, d[full][v]);
    return best;                                              // INF = impossible
}
