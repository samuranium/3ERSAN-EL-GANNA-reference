// ============================================================================
// DIJKSTRA -- single source shortest path, O((V + E) log V)
// ----------------------------------------------------------------------------
// WHEN: one source, every weight >= 0. That is the whole condition.
//
// NOT THIS -- check before you type:
//   any negative edge          -> Bellman-Ford / SPFA (also detects neg cycle)
//                                 or potentials, see [R6] below
//   all weights 0 or 1         -> 0-1 BFS, deque, O(V + E). Strictly better
//   all weights equal          -> plain BFS
//   all pairs, n <= ~500       -> Floyd-Warshall, ../Floyd-Warshall.cpp
//   dense, E ~ V^2             -> O(V^2) scan without a heap (prim mst.cpp shape)
//   at most k edges on path    -> Bellman-Ford by layers, or state [R3]
//
// PITFALLS:
//   d[] must be ll. 2e5 edges * 1e9 overflows int, and it reads as a WA on
//     test 40, not as an overflow.
//   The `du > d[u]` line is not an optimisation -- it is what makes this
//     O(E log V). Drop it and stale heap entries get re-expanded.
//   priority_queue is a MAX-heap by default; greater<> makes it a min-heap.
//   Don't use set<> as the queue unless you genuinely need erase/decrease-key.
//     Lazy deletion below is faster and shorter.
//   Nodes are 1-indexed here. Unreachable stays INF; par[] is the SP tree.
//
// ----------------------------------------------------------------------------
// GRAPH REWRITES -- change the graph, not the algorithm. This is where most
// Dijkstra problems actually live.
// ----------------------------------------------------------------------------
// [R1] EVERY NODE -> ONE TARGET t.  Reverse every edge, run Dijkstra from t.
//      Result dr[v] = dist(v -> t) in the original graph. On an undirected
//      graph reversing is a no-op, so this only matters when directed.
//
//      Run BOTH (ds from s on g, dt from t on reversed g) and you get:
//        best path s->t THROUGH v        ds[v] + dt[v]
//        is edge (u,v,w) on SOME s-t SP  ds[u] + w + dt[v] == ds[t]
//        shortest path DAG               keep exactly those edges, then DP
//                                        on it for counting / lex-smallest
//        best if you may delete one edge  recompute over non-tree edges
//
// [R2] MULTI-SOURCE.  Push every source with d = 0 before the loop. Same as a
//      virtual node joined to all of them by 0-weight edges, but free.
//      "distance to the NEAREST special node" is exactly this.
//      Sources with an entry cost c_i: push {c_i, i} instead of {0, i}.
//      A virtual SINK is the mirror -- add t' with 0-edges from every sink.
//
// [R3] STATE EXPANSION.  Index d[] by (node, state) and change nothing else:
//        fuel left, keys held, #free upgrades used, parity of edge count,
//        "may skip one edge", last edge's colour.
//      Cost: V * S heap entries -- budget it before committing.
//
// [R4] WEIGHT ON NODES, not edges. Split u into u_in -> u_out with the node
//      weight on that edge; redirect incoming to u_in, outgoing from u_out.
//      Or, if all paths end at a node: just add w[v] when relaxing into v.
//
// [R5] LAYERED / TIME-EXPANDED graphs. Timetables, "edge usable only at time
//      t", teleporters with cooldown: node = (place, time mod k).
//
// [R6] NEGATIVE EDGES, NO NEGATIVE CYCLE -- Johnson potentials. Bellman-Ford
//      once from a virtual node joined to all (h[]), then reweight
//        w'(u,v) = w(u,v) + h[u] - h[v]   >= 0
//      and Dijkstra on w'. Recover true dist as d'[v] - h[s] + h[v].
//      This is the same trick that makes Dijkstra-based MCMF work.
//
// ----------------------------------------------------------------------------
// RIDERS -- one line each on top of the relax step, graph unchanged.
// ----------------------------------------------------------------------------
//   count of shortest paths   d[u]+w <  d[v] -> cnt[v] = cnt[u]
//                             d[u]+w == d[v] -> cnt[v] += cnt[u]   (mod it)
//   min #edges among ties     compare pair (dist, hops) lexicographically
//   minimise the MAX edge     relax with max(d[u], w) instead of d[u] + w
//   maximise the MIN edge     relax with min(d[u], w) and a MAX-heap
//                             (both also fall out of MST -- see MST/ folder)
//   maximise a PRODUCT of     weights in [0,1]: multiply and use a max-heap,
//     probabilities           or take -log and run normal Dijkstra
//   second shortest path      keep the two best values per node, pop until a
//                             node has been settled twice
//
// SPEED, if it ever matters: weights bounded by small C -> Dial's algorithm,
//   C+1 buckets instead of a heap, O(V*C + E).
// ============================================================================
const int N = 200'123;
const ll INF = 4e18;
vector<pair<int, ll>> g[N];   // (to, w)
ll d[N];
int par[N];

void dijkstra(int s, int n) {
    for (int i = 1; i <= n; i++) d[i] = INF, par[i] = -1;
    priority_queue<pair<ll, int>, vector<pair<ll, int>>, greater<>> pq;
    d[s] = 0, pq.push({0, s});
    while (pq.size()) {
        auto [du, u] = pq.top(); pq.pop();
        if (du > d[u]) continue;              // stale entry, already improved
        for (auto &[v, w] : g[u])
            if (du + w < d[v])
                d[v] = du + w, par[v] = u, pq.push({d[v], v});
    }
}

// s -> t node list, empty if unreachable. Needs par[] from the run above.
vector<int> path(int t) {
    if (d[t] == INF) return {};
    vector<int> p;
    for (int u = t; u != -1; u = par[u]) p.push_back(u);
    reverse(p.begin(), p.end());
    return p;
}

// [R1] reversed copy, for "every node -> t" and the ds/dt pair tricks.
vector<pair<int, ll>> rg[N];
void build_rev(int n) {
    for (int u = 1; u <= n; u++)
        for (auto &[v, w] : g[u]) rg[v].push_back({u, w});
}
