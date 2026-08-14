// ============================================================================
// K SHORTEST PATHS -- the k smallest s->t route lengths, O(k*E log(k*E))
// ----------------------------------------------------------------------------
// ############################################################################
// #  READ THIS BEFORE USING IT: WALKS, NOT SIMPLE PATHS
// #
// #  This returns the k shortest WALKS -- a route may revisit a vertex or an
// #  edge. That is what almost every competitive problem means by "k shortest
// #  routes" (CSES Flight Routes is exactly this), and it is why the code is
// #  fifteen lines.
// #
// #  k shortest SIMPLE paths (no repeats) is a different, much heavier problem
// #  -> Yen's algorithm, O(k*V*(E + V log V)), or Eppstein. If the statement
// #  forbids revisiting, this file is the WRONG answer and will silently give
// #  you too-small values.
// ############################################################################
//
// HOW: Dijkstra, but stop discarding. Let every vertex be settled up to k times
//   instead of once, and collect a result each time t comes off the heap. The
//   i-th time t is popped, that is the i-th shortest walk -- the heap pops in
//   nondecreasing order, so they come out already sorted.
//
// WHY cnt[v] < k IS THE WHOLE ALGORITHM: past k settlements a vertex can no
//   longer contribute to any of the k best routes to t, since any route through
//   it would be at least as long as k routes already found. Dropping that guard
//   makes the heap explode.
//
// PITFALLS:
//   Weights must be >= 0 -- same requirement as Dijkstra.
//   Values repeat. If the problem wants k DISTINCT lengths, dedupe as you
//     collect and keep going until you have k distinct ones.
//   Fewer than k routes may exist; the result can be shorter than k.
//   Heap size is O(k*E) -- with k = 1e5 and E = 1e5 that is 1e10 pushes, which
//     will not fit. The guard keeps it to O(k*E) total, so budget k*E.
//   ll for distances.
//   s == t: the empty walk of length 0 counts or not depending on the problem.
//     This code does count it, since t is popped immediately at distance 0.
//
// VARIANTS:
//   just the k-th value          run with this k, take res.back()
//   k shortest per vertex        collect on every pop, not only at t
//   second shortest walk         k = 2 (and note it may equal the shortest)
//   second shortest SIMPLE path  not this -- Yen, or "delete one edge of the
//                                shortest path and re-run" for the easy version
//   k shortest with a constraint push the state into the node, see dijkstra.cpp [R3]
// ============================================================================
const int N = 200'123;
vector<pair<int, ll>> g[N];   // (to, w), w >= 0

// the k smallest s->t walk lengths, ascending. May return fewer than k.
vector<ll> k_shortest(int s, int t, int n, int k) {
    vector<int> cnt(n + 1, 0);
    vector<ll> res;
    priority_queue<pair<ll, int>, vector<pair<ll, int>>, greater<>> pq;
    pq.push({0, s});
    while (pq.size() && (int)res.size() < k) {
        auto [d, u] = pq.top(); pq.pop();
        if (cnt[u] >= k) continue;          // this vertex can no longer help
        cnt[u]++;
        if (u == t) res.push_back(d);
        for (auto &[v, w] : g[u])
            if (cnt[v] < k) pq.push({d + w, v});
    }
    return res;
}
