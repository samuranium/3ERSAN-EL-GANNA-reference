// ============================================================================
// MINIMUM MEAN CYCLE (Karp) -- minimise (total weight) / (number of edges)
// ----------------------------------------------------------------------------
// "Minimise the cost PER STEP around a cycle." Karp's characterisation:
//
//     d[k][v] = min weight of a walk of EXACTLY k edges ending at v
//
//     answer = min over v of  max over k in [0, n-1] of
//                  ( d[n][v] - d[k][v] ) / ( n - k )
//
// O(V*E) time, O(V^2) memory. The `max` inside is not a typo -- it is what
// makes the expression a valid lower bound for every cycle through v, and the
// outer `min` then finds the tight one.
//
// ############################################################################
// #  IT IS A WALK OF EXACTLY k EDGES, NOT A SHORTEST PATH
// #
// #  d[k][v] allows repeated vertices and repeated edges. Initialise
// #  d[0][v] = 0 for EVERY v (not just a source) -- Karp's proof needs walks
// #  that may start anywhere. Using d[0][s] = 0 and INF elsewhere computes
// #  something else entirely and reports a mean that is too large.
// ############################################################################
//
// ############################################################################
// #  THE GRAPH MUST BE STRONGLY CONNECTED, OR RESTRICT TO ONE SCC
// #
// #  Karp's formula assumes every vertex lies on some cycle reachable from
// #  every other. On a graph with several components, run it per SCC and take
// #  the best -- graph/SCC/tarjan - bridges.cpp for the condensation.
// #  On a graph with no cycle at all, every d[n][v] stays INF; return +INF.
// ############################################################################
//
// PITFALLS:
//   The answer is a FRACTION. Compare candidates by cross-multiplication
//     (a1*b2 vs a2*b1) rather than dividing, or accept the double.
//   d has n+1 rows -- indices 0..n. Sizing it n is an out-of-bounds write.
//   Skip relaxations from INF: `if (d[k][u] == INF) continue;`.
//   Weights may be negative; that is the usual reason this is asked.
//   The BINARY SEARCH alternative is often easier to write and adapt: guess
//     lambda, subtract it from every edge, and test for a negative cycle with
//     Bellman-Ford. It is O(V*E*log(range/eps)) but reuses code you have.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   minimise average edge weight around a cycle       min_mean_cycle()
//   MAXIMISE the average                              negate every weight
//   "minimum cost per unit time" / "best ratio loop"   this
//   "profit per day of a repeating schedule"           this, weights =
//                                                      cost - profit
//   maximise (sum of a) / (sum of b) over a cycle      binary search t, use
//                                                      weight a - t*b, test
//                                                      for a positive cycle
//                                                      (Dinkelbach)
//   any "maximise a ratio" over a structure            binary search the ratio
//                                                      and test the sign --
//                                                      the general pattern
//   does ANY negative cycle exist                      Bellman-Ford with a
//                                                      virtual source --
//                                                      graph/shortest path/
//                                                      bellman ford.cpp
//   recover the actual cycle                           track the argmin
//                                                      predecessor in d[][],
//                                                      then walk back n steps
//                                                      and find the repeat
//
// NOT THIS:
//   plain "is there a negative cycle"  -> Bellman-Ford, much cheaper.
//   minimise the TOTAL weight of a cycle -> that is a shortest-cycle problem,
//     not a mean one.
//   the ratio is over a PATH, not a cycle -> binary search + shortest path.
// ============================================================================
const ll INF = 1e18 / 4;

int n;
vector<array<ll, 3>> edges;             // {u, v, w}

// returns the minimum cycle mean as a fraction (num, den), or {INF, 1} if the
// graph has no cycle. Assumes vertices 0..n-1 and a strongly connected graph.
pair<ll, ll> min_mean_cycle() {
    vector<vector<ll>> d(n + 1, vector<ll>(n, INF));
    for (int v = 0; v < n; v++) d[0][v] = 0;         // ALL vertices, not one
    for (int k = 0; k < n; k++)
        for (auto &e : edges) {
            if (d[k][e[0]] == INF) continue;
            d[k + 1][e[1]] = min(d[k + 1][e[1]], d[k][e[0]] + e[2]);
        }
    ll bn = INF, bd = 1;                              // best as a fraction
    for (int v = 0; v < n; v++) {
        if (d[n][v] == INF) continue;
        ll cn = -INF, cd = 1;                         // max over k, per vertex
        bool ok = false;
        for (int k = 0; k < n; k++) {
            if (d[k][v] == INF) continue;
            ll num = d[n][v] - d[k][v], den = n - k;
            if (!ok || num * cd > cn * den) cn = num, cd = den, ok = true;
        }
        if (!ok) continue;
        if (bn == INF || cn * bd < bn * cd) bn = cn, bd = cd;
    }
    return {bn, bd};
}

////////////////////////////////////////////////////////////////////////////////
// THE BINARY SEARCH ALTERNATIVE -- shorter, adapts to any ratio, reuses
// Bellman-Ford. Subtract lambda from every edge; a negative cycle exists iff
// the minimum mean is < lambda.
//
// Integer weights: the answer is p/q with q <= n, so distinct means differ by
// at least 1/n^2 -- about 60 iterations of bisection on doubles is plenty,
// or use Stern-Brocot to land on the exact fraction.
////////////////////////////////////////////////////////////////////////////////
bool has_negative_cycle(double lambda) {
    vector<double> d(n, 0.0);                         // virtual source: all 0
    for (int i = 0; i < n; i++)
        for (auto &e : edges)
            d[e[1]] = min(d[e[1]], d[e[0]] + (double)e[2] - lambda);
    for (auto &e : edges)
        if (d[e[0]] + (double)e[2] - lambda < d[e[1]] - 1e-9) return true;
    return false;
}
double min_mean_cycle_bs(double lo, double hi) {
    for (int it = 0; it < 60; it++) {
        double mid = (lo + hi) / 2;
        if (has_negative_cycle(mid)) hi = mid;
        else lo = mid;
    }
    return hi;
}
