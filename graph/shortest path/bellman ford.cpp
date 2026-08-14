// ============================================================================
// BELLMAN-FORD -- shortest path WITH negative edges, O(V * E)
//                 + negative cycle detection and extraction
// ----------------------------------------------------------------------------
// WHEN: any weight may be negative. That is the only reason to be here --
//   if every weight is >= 0, Dijkstra is faster and shorter. ./dijkstra.cpp
//
// WHY V-1 PASSES: a shortest path has at most V-1 edges, and after pass k every
//   shortest path using <= k edges is final. So if pass V still improves
//   something, that improvement used V edges, which forces a repeated vertex --
//   a cycle -- and it only helped because the cycle is negative.
//
// ############################################################################
// #  THE PITFALL THAT ACTUALLY BITES
// #
// #      if (d[u] < INF && d[u] + w < d[v])
// #           ^^^^^^^^^^^^ this guard is not optional
// #
// #  Without it you relax from unreachable nodes: INF + (-5) < INF is TRUE, so
// #  phantom distances spread from vertices you can never reach, and with
// #  INF = 4e18 the addition also signed-overflows.
// #  SYMPTOM: answers that are slightly too small, or garbage near 9e18, on
// #  disconnected graphs only -- so the samples pass.
// ############################################################################
//
// OTHER PITFALLS:
//   Needs an EDGE LIST, not adjacency. That is the whole data structure.
//   d must be ll.
//   V-1 passes for distances; the V-th pass is the detector. Off-by-one here
//     means you either miss cycles or report cycles that are not there.
//   The early break (no relaxation this pass) is free and usually huge.
//   Undirected negative edges are an instant negative cycle -- u->v->u. If the
//     statement gives them, you are modelling something else (matching, DP).
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
// [1] IS THERE ANY NEGATIVE CYCLE, anywhere, not just reachable from s?
//     Add a virtual node 0 with a 0-weight edge to every vertex and run from 0.
//     Everything is reachable, so nothing hides in another component.
//
// [2] EXTRACT THE CYCLE -- get_cycle() below. Walk par[] n times first: that
//     lands you strictly inside the cycle no matter where the relaxation was
//     detected. Then follow par[] until you return to the same node.
//
// [3] SHORTEST PATH USING AT MOST k EDGES. Run exactly k passes, but relax from
//     a SNAPSHOT of d taken before the pass:
//         auto prev = d;  for each edge: d[v] = min(d[v], prev[u] + w);
//     Without the snapshot one pass can chain several edges and you silently
//     allow more than k. This also makes negative cycles harmless -- k passes
//     is k passes.
//
// [4] JOHNSON POTENTIALS, the reason dijkstra.cpp [R6] points here. Run [1] to
//     get h[] from the virtual source. Then w'(u,v) = w + h[u] - h[v] >= 0 by
//     the triangle inequality, so Dijkstra is legal on w'. Recover the real
//     distance as d'[v] - h[s] + h[v]. Same trick makes Dijkstra-based MCMF
//     work -- see graph/flows/.
//
// [5] DIFFERENCE CONSTRAINTS -- the one people miss. A system of
//         x_v - x_u <= c
//     is exactly an edge u -> v of weight c. Add the virtual source [1], run
//     Bellman-Ford, and d[] IS a valid assignment. Infeasible iff there is a
//     negative cycle. Handles scheduling, "b is at least k after a", and any
//     pile of pairwise inequalities.
//     x_v - x_u >= c  ->  rewrite as x_u - x_v <= -c.
//     x_v - x_u == c  ->  both directions.
//
// [6] NEGATIVE INFINITY NODES. d[t] is meaningless if t is reachable FROM a
//     negative cycle. After detection, mark every node reachable from any
//     cycle node (one BFS/DFS) and report -INF for those.
//
// [7] ARBITRAGE / PRODUCT CYCLES. A cycle whose weights MULTIPLY to > 1 becomes
//     a negative cycle under w -> -log(w). Currency exchange, probability
//     ratios. Watch precision: use long double and compare against -1e-9.
//
// [8] MINIMUM MEAN CYCLE -> Karp's algorithm, O(V*E), different recurrence.
//     Not this file, but this is where you will start looking.
//
// SPFA (queue-based Bellman-Ford) is much faster in practice and worst-case
//   identical, but it is deliberately hackable on Codeforces. If you want it,
//   graph/flows/MCMF.cpp already contains a deque SPFA with the SLF heuristic.
// ============================================================================
const int N = 200'123;
const ll INF = 4e18;
struct E { int u, v; ll w; };
vector<E> e;
ll d[N];
int par[N];

// Returns -1 if no negative cycle was reached, otherwise a vertex whose
// distance still improved on the n-th pass (use get_cycle on it).
int bellman(int s, int n) {
    for (int i = 1; i <= n; i++) d[i] = INF, par[i] = -1;
    d[s] = 0;
    int x = -1;
    for (int it = 1; it <= n; it++) {
        x = -1;
        for (auto &[u, v, w] : e)
            if (d[u] < INF && d[u] + w < d[v])
                d[v] = d[u] + w, par[v] = u, x = v;
        if (x == -1) break;               // nothing moved: distances are final
    }
    return x;                             // != -1 after pass n  =>  negative cycle
}

// [2] the cycle itself, in order. Pass the x returned by bellman().
vector<int> get_cycle(int x, int n) {
    for (int i = 0; i < n; i++) x = par[x];   // n steps back lands inside the cycle
    vector<int> c;
    for (int v = x;; v = par[v]) {
        c.push_back(v);
        if (v == x && c.size() > 1) break;
    }
    reverse(c.begin(), c.end());
    return c;
}
