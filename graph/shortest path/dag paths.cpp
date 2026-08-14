// ============================================================================
// TOPOLOGICAL SORT (Kahn) + SHORTEST / LONGEST PATH ON A DAG, O(V + E)
// ----------------------------------------------------------------------------
// Two things in one file because the second is three lines once you have the
// first: process vertices in topological order and every predecessor of u is
// already final when you reach u. No heap, no relaxation rounds.
//
// ############################################################################
// #  THE REASON THIS FILE MATTERS
// #
// #  LONGEST PATH IS NP-HARD ON A GENERAL GRAPH, AND LINEAR ON A DAG.
// #
// #  Dijkstra cannot do longest (negate the weights and it is negative-edge),
// #  Bellman-Ford cannot (a positive cycle makes it unbounded). On a DAG there
// #  are no cycles, so max works exactly like min. If a statement asks for a
// #  longest / maximum-weight path, your first question is "is it acyclic".
// #  If it is not, look for an SCC condensation -- that IS a DAG.
// ############################################################################
//
// ALSO: shortest path on a DAG accepts NEGATIVE weights at no cost. No cycles
//   means no negative cycles. Do not reach for Bellman-Ford on a DAG.
//
// PITFALLS:
//   Seed the queue with EVERY indegree-0 vertex, not just the source.
//   ord.size() != n after Kahn  <=>  the graph has a cycle. Free detection --
//     this is the cheapest cycle test for a directed graph.
//   Skip unreachable u (d[u] == sentinel) before relaxing out of it, exactly
//     as in bellman ford.cpp -- otherwise the sentinel propagates.
//   Longest path uses -INF as the sentinel, not 0. With 0 you silently allow
//     paths to start anywhere.
//   Multi-test: clear g[], indeg[] and ord for 1..n.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
// [1] CYCLE DETECTION, directed. Run topo(); false means a cycle exists. The
//     vertices left with indeg > 0 are exactly those on or downstream of it.
//
// [2] LEXICOGRAPHICALLY SMALLEST topological order. Swap queue for
//       priority_queue<int, vector<int>, greater<>>
//     and nothing else changes. O((V+E) log V). Note this is NOT the same as
//     "smallest permutation" for every problem -- read the statement.
//
// [3] LONGEST PATH / CRITICAL PATH. dag_dist(s, n, 1). Project scheduling is
//     this: tasks are vertices, dependencies are edges, the critical path is
//     the longest one and it is the minimum completion time.
//     For longest path over the WHOLE dag (any start), seed every source at 0.
//
// [4] COUNT PATHS s -> v. Same sweep, cnt[s] = 1, and in topo order
//       for u in ord:  for (v, w) in g[u]:  cnt[v] += cnt[u]        (mod it)
//     Count of paths between all pairs is V sweeps, or bitset reachability if
//     you only need yes/no -- see floyd warshall.cpp [3].
//
// [5] DP ON A DAG generally. Every DP is a DAG over its states, and the
//     topological order IS the evaluation order. When a memoised recursion is
//     hard to convert to loops, this is the conversion.
//
// [6] THE GRAPH HAS CYCLES BUT YOU WANT THIS ANYWAY -> condense the SCCs first
//     (graph/SCC/tarjan - bridges.cpp, build_dag). The condensation is a DAG,
//     and Tarjan already hands you the components in reverse topological order,
//     so you can skip Kahn entirely -- just iterate component ids downward.
//
// [7] MINIMUM PATH COVER of a DAG (fewest vertex-disjoint paths covering all
//     vertices) = n - (maximum bipartite matching on the split graph). Not this
//     file -- graph/flows/.
//
// [8] NUMBER OF DISTINCT TOPOLOGICAL ORDERS is #P-hard in general. For n <= 20
//     it is a subset DP: f[mask] = sum over v in mask with no predecessor left.
// ============================================================================
const int N = 200'123;
const ll INF = 4e18;
vector<pair<int, ll>> g[N];   // (to, w); drop the weight if you only need order
int indeg[N];
vector<int> ord;

// false => there is a cycle (ord then holds only the acyclic prefix)
bool topo(int n) {
    for (int i = 1; i <= n; i++) indeg[i] = 0;
    for (int u = 1; u <= n; u++)
        for (auto &[v, w] : g[u]) indeg[v]++;
    queue<int> q;                    // [2] priority_queue + greater<> for lex-smallest
    for (int i = 1; i <= n; i++)
        if (!indeg[i]) q.push(i);
    ord.clear();
    while (q.size()) {
        int u = q.front(); q.pop();
        ord.push_back(u);
        for (auto &[v, w] : g[u])
            if (--indeg[v] == 0) q.push(v);
    }
    return (int)ord.size() == n;
}

ll d[N];
// longest = 1 for the longest path. Negative weights are fine either way.
// Requires topo() to have run. Unreachable vertices keep the sentinel.
void dag_dist(int s, int n, bool longest) {
    ll SENT = longest ? -INF : INF;
    for (int i = 1; i <= n; i++) d[i] = SENT;
    d[s] = 0;
    for (int u : ord)
        if (d[u] != SENT)
            for (auto &[v, w] : g[u])
                if (longest ? d[u] + w > d[v] : d[u] + w < d[v])
                    d[v] = d[u] + w;
}
