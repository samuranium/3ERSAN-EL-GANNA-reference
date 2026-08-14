// ============================================================================
// FLOYD-WARSHALL -- all pairs shortest path, O(V^3), negative edges OK
// ----------------------------------------------------------------------------
// WHEN: you need EVERY pair, and V is small. V <= 500 is 1.25e8, comfortable.
//   V = 1000 is 1e9 -- only with a 3s+ limit and the row-skip below.
//   Need one source only? Dijkstra / Bellman-Ford are orders faster.
//   V large but E small, all weights >= 0? Run Dijkstra from each node:
//     O(V * E log V) beats V^3 on sparse graphs.
//
// READ IT AS A DP: d[k][i][j] = best i->j using only 1..k as intermediates.
//   Either you use k or you don't: min(d[k-1][i][j], d[k-1][i][k] + d[k-1][k][j]).
//   The k layer can be dropped in place -- the two terms it reads are already
//   correct for layer k, because d[i][k] and d[k][j] never improve at step k.
//
// ############################################################################
// #  TWO WAYS TO GET THIS WRONG, BOTH SILENT
// #
// #  1. k MUST BE THE OUTERMOST LOOP. With k inner the DP reads layers that do
// #     not exist yet. It still terminates and still looks plausible -- it is
// #     just wrong, and only on graphs where a 3+ edge path wins.
// #
// #  2. GUARD THE INF. d[i][k] + d[k][j] with an INF sentinel invents paths
// #     through unreachable vertices, and with a big INF it also overflows.
// #     The old graph/Floyd-Warshall.cpp had neither guard.
// ############################################################################
//
// OTHER PITFALLS:
//   d[i][i] = 0 before you start, and parallel edges must take min, not last.
//   Directed by default. Undirected = add both directions at init.
//   A negative SELF loop is a negative cycle; it will show up as d[i][i] < 0.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
// [1] NEGATIVE CYCLE ANYWHERE: after the run, d[i][i] < 0 for some i.
//     Which PAIRS are ruined: d[i][j] is -infinity iff there is some k with
//         d[i][k] < INF && d[k][k] < 0 && d[k][j] < INF
//     One extra O(V^3) sweep to mark them, or O(V^2) per bad k.
//
// [2] PATH RECONSTRUCTION: nxt[i][j] = first hop. Set nxt[i][j] = j at init,
//     and on every relaxation nxt[i][j] = nxt[i][k]. path() below.
//
// [3] TRANSITIVE CLOSURE / REACHABILITY: same triple loop with bool, or
//     bitset<N> reach[N] and  if (reach[i][k]) reach[i] |= reach[k];
//     That is O(V^3 / 64) and makes V = 2000 practical.
//
// [4] BOTTLENECK (minimax) PATH -- minimise the largest edge on the path:
//         d[i][j] = min(d[i][j], max(d[i][k], d[k][j]))
//     Widest path (maximise the smallest edge) is the mirror: max of min.
//     For a single pair on an undirected graph, MST is faster -- MST/ folder.
//
// [5] GRAPH DIAMETER / ECCENTRICITY: max over finite d[i][j], and
//     ecc[i] = max_j d[i][j]. Centre = argmin ecc.
//
// [6] IS EDGE (u,v,w) ON SOME SHORTEST PATH i->j:
//         d[i][u] + w + d[v][j] == d[i][j]
//     Same identity as the ds/dt trick in dijkstra.cpp [R1], but you already
//     have every pair, so no reversed graph is needed.
//
// [7] ADDING A VERTEX ONLINE: when vertex k arrives, run just the i,j double
//     loop for that one k. O(V^2) per insertion instead of a full rebuild.
//     Deleting a vertex does not work this way -- rebuild.
//
// [8] EXACTLY k EDGES / min path of length k -> min-plus matrix power, not
//     this. See DP/matrix/minlenK.cpp.
//
// [9] COUNTING shortest paths: keep cnt[i][j]; on a strict improvement copy
//     cnt[i][k]*cnt[k][j], on a tie add it. Mod it.
// ============================================================================
const int N = 512;
const ll INF = 4e18;
ll d[N][N];
int nxt[N][N];

void init(int n) {
    for (int i = 1; i <= n; i++)
        for (int j = 1; j <= n; j++)
            d[i][j] = (i == j ? 0 : INF), nxt[i][j] = j;
}
// edges:  d[u][v] = min(d[u][v], w);   (and d[v][u] too if undirected)

void floyd(int n) {
    for (int k = 1; k <= n; k++)            // k OUTERMOST -- see the box above
        for (int i = 1; i <= n; i++) {
            if (d[i][k] == INF) continue;   // row skip: correctness + big speedup
            for (int j = 1; j <= n; j++)
                if (d[k][j] < INF && d[i][k] + d[k][j] < d[i][j])
                    d[i][j] = d[i][k] + d[k][j], nxt[i][j] = nxt[i][k];
        }
}
// [2] node list i -> j, empty if unreachable. Needs nxt[] from the run above.
vector<int> path(int u, int v) {
    if (d[u][v] == INF) return {};
    vector<int> p{u};
    while (u != v) u = nxt[u][v], p.push_back(u);
    return p;
}
// [1] true if the graph has a negative cycle
bool neg_cycle(int n) {
    for (int i = 1; i <= n; i++) if (d[i][i] < 0) return true;
    return false;
}

// ////////////////////////////////////////////////////////////////////////////
// [3] TRANSITIVE CLOSURE with bitset -- O(V^3 / 64), so V = 2000 is fine
// ----------------------------------------------------------------------------
// Reachability is boolean, so the inner j loop is 64 independent OR operations
// per word -- the whole loop becomes one |=. Nothing else changes: k is still
// outermost, and it is still the same DP.
//
// Distances CANNOT be done this way; the min-plus inner loop has no word-level
// equivalent. This trick is only for "is j reachable from i".
//
// INIT:
//   for each i           reach[i][i] = 1        <- reflexive: 0-length path
//   for each edge u->v   reach[u][v] = 1
//
//   If the question is "can i reach j using AT LEAST ONE edge", leave the
//   diagonal clear at init. reach[i][i] then ends up 1 exactly when i lies on
//   a cycle, which is usually what such a problem is really asking.
//
// MEMORY: NB^2 bits. NB = 2048 is 512 KB -- free compared to ll d[N][N].
//
// USES: DAG reachability, "how many pairs are connected", implication closure
//   for small 2-SAT, and any boolean O(n^3) triple loop -- the same |= rewrite
//   applies whenever the innermost operation is an OR.
// ////////////////////////////////////////////////////////////////////////////
const int NB = 2048;
bitset<NB> reach[NB];

void closure(int n) {
    for (int k = 1; k <= n; k++)
        for (int i = 1; i <= n; i++)
            if (reach[i][k]) reach[i] |= reach[k];
}
// # of ordered reachable pairs:  sum over i of reach[i].count()
