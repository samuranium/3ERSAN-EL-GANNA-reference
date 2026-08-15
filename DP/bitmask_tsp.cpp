// ============================================================================
// HELD-KARP -- TSP and general "visit every element once" DP, O(2^n * n^2)
// ----------------------------------------------------------------------------
// dp[mask][last] = cheapest way to have visited exactly `mask`, standing at
// `last`. The state is (what is done, where I am) -- and almost every bitmask
// DP is that same pair.
//
// ############################################################################
// #  n IS ABOUT 20, AND THE MEMORY IS WHAT STOPS YOU
// #
// #      2^n * n ints:  n=20 -> 84 MB      n=18 -> 19 MB      n=22 -> 369 MB
// #
// #  Time 2^n*n^2 at n=20 is 4.2e8 -- tight but passable. Memory is the wall.
// #  If n is 22-24 and the SECOND dimension is not needed (order-independent
// #  cost), drop `last` and it becomes 2^n, which is a different, much cheaper
// #  DP -- see [1] below.
// ############################################################################
//
// ############################################################################
// #  PATH vs CYCLE -- decide before writing the answer line
// #
// #    CYCLE (return to start):  answer = min over last of dp[full][last] + c[last][0]
// #    PATH  (end anywhere):     answer = min over last of dp[full][last]
// #    PATH with fixed ends:     start the DP at s, take dp[full][t]
// #
// #  Fixing vertex 0 as the start is free for a CYCLE (every cycle passes
// #  through it) and WRONG for a path, which must try every start -- or add a
// #  dummy node at distance 0 to everything.
// ############################################################################
//
// PITFALLS:
//   The transition must check `mask >> v & 1` is 0 before moving to v; adding
//     an already-visited vertex silently produces a shorter, invalid tour.
//   Unreachable states must stay INF and be skipped, or INF + cost overflows.
//   Iterating masks in increasing order is required -- a mask only ever
//     transitions to a strictly larger mask, so plain ascending order works.
//   For reconstruction store the parent, or re-walk the DP backwards.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS -- the shapes that are this DP in disguise
// ----------------------------------------------------------------------------
// [1] ORDER DOES NOT MATTER -> drop `last`, dp[mask] only, O(2^n * n).
//     "assign n tasks to n workers", "partition into groups", "cover all with
//     minimum cost". Iterate submasks for partitioning: O(3^n).
//
// [2] ASSIGNMENT dp[mask] where popcount(mask) IS the row index -- you never
//     need a second dimension because the row is implied. Classic for
//     "n people, n jobs, cost matrix", though Hungarian is O(n^3) and better
//     past n = 20: graph/matching/hungarian.cpp.
//
// [3] COUNTING Hamiltonian paths / cycles: same DP with += instead of min.
//
// [4] SHORTEST SUPERSTRING: overlap[i][j] = how much j saves after i, then
//     TSP on the overlap matrix. Same for "merge all strings".
//
// [5] TSP WITH REPEATS ALLOWED: run Floyd-Warshall first so c[][] becomes
//     shortest paths, then this. Otherwise the "visit once" constraint is
//     wrong for the real graph.
//
// [6] n UP TO 40: meet in the middle. Split, enumerate 2^(n/2) each side.
//     Held-Karp does not stretch that far.
//
// [7] BITMASK OVER SOMETHING ELSE than vertices -- primes used, colours seen,
//     broken profile of a grid row (DP/broken_profile_dp.cpp). Same machinery.
//
// NOT THIS: n > 24 -> no exact bitmask solution; look for structure (a tree, a
//   DAG, a metric with special properties) or an approximation.
// ============================================================================
const int TN = 20;
ll dp[1 << TN][TN], c[TN][TN];
const ll TINF = (ll)4e18 / 4;

// cheapest Hamiltonian CYCLE through all n vertices, starting and ending at 0
ll tsp_cycle(int n) {
    int full = 1 << n;
    for (int m = 0; m < full; m++)
        for (int v = 0; v < n; v++) dp[m][v] = TINF;
    dp[1][0] = 0;                                  // start at 0, only 0 visited
    for (int m = 1; m < full; m++)
        for (int u = 0; u < n; u++) {
            if (dp[m][u] >= TINF || !(m >> u & 1)) continue;
            for (int v = 0; v < n; v++) {
                if (m >> v & 1) continue;          // already visited
                ll nd = dp[m][u] + c[u][v];
                if (nd < dp[m | 1 << v][v]) dp[m | 1 << v][v] = nd;
            }
        }
    ll best = TINF;
    for (int v = 0; v < n; v++)
        if (dp[full - 1][v] < TINF) best = min(best, dp[full - 1][v] + c[v][0]);
    return best;
}
// cheapest Hamiltonian PATH, any start, any end
ll tsp_path(int n) {
    int full = 1 << n;
    for (int m = 0; m < full; m++)
        for (int v = 0; v < n; v++) dp[m][v] = TINF;
    for (int v = 0; v < n; v++) dp[1 << v][v] = 0;  // every start, see the box
    for (int m = 1; m < full; m++)
        for (int u = 0; u < n; u++) {
            if (dp[m][u] >= TINF || !(m >> u & 1)) continue;
            for (int v = 0; v < n; v++) {
                if (m >> v & 1) continue;
                ll nd = dp[m][u] + c[u][v];
                if (nd < dp[m | 1 << v][v]) dp[m | 1 << v][v] = nd;
            }
        }
    ll best = TINF;
    for (int v = 0; v < n; v++) best = min(best, dp[full - 1][v]);
    return best;
}
// the tour itself, for the cycle version. Call after tsp_cycle.
vector<int> tsp_order(int n) {
    int full = (1 << n) - 1, m = full, cur = -1;
    ll best = TINF;
    for (int v = 0; v < n; v++)
        if (dp[full][v] < TINF && dp[full][v] + c[v][0] < best)
            best = dp[full][v] + c[v][0], cur = v;
    vector<int> path;
    while (cur != -1) {
        path.push_back(cur);
        int pm = m ^ (1 << cur), prev = -1;
        if (pm)
            for (int u = 0; u < n; u++)
                if ((pm >> u & 1) && dp[pm][u] < TINF && dp[pm][u] + c[u][cur] == dp[m][cur]) {
                    prev = u;
                    break;
                }
        m = pm, cur = prev;
    }
    reverse(path.begin(), path.end());
    return path;
}
