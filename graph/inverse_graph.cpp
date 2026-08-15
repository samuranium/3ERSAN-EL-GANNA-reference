// ============================================================================
// COMPLEMENT GRAPH TRAVERSAL -- BFS the graph you were NOT given, O(n + m)
// ----------------------------------------------------------------------------
// The complement of a graph with n = 1e5 and m = 1e5 edges has ~5e9 edges. You
// cannot build it. But you can WALK it in O(n + m) total, which is enough for
// connected components, BFS distances, and bipartiteness on the complement.
//
// ############################################################################
// #  THE TRICK: KEEP THE UNVISITED VERTICES IN A SET
// #
// #  From u, the complement-neighbours are "every unvisited vertex that is NOT
// #  a real neighbour of u". So:
// #     walk the unvisited set, skip the ones adjacent to u in the REAL graph,
// #     take the rest, and ERASE them from the set.
// #
// #  Each vertex leaves the set once, so the total work over the whole BFS is
// #  O(n log n) for the erases plus O(m) for the skips -- not O(n^2). The skip
// #  cost is bounded by the REAL edge count because a vertex is only skipped
// #  when a real edge explains it.
// ############################################################################
//
// PITFALLS:
//   You must ERASE from the set as you enqueue, not after the loop. Leaving a
//     vertex in makes it rediscovered by every later u and the bound collapses
//     to O(n^2).
//   Collect the survivors into a temporary vector BEFORE erasing, or you
//     invalidate the iterator you are walking.
//   `adj` must support O(1) or O(log) membership -- a sorted vector with
//     binary_search, or an unordered_set. A plain vector scan reintroduces the
//     O(n·deg) you were avoiding.
//   Vertices are 0-indexed here.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   connected components of the complement    comp_components()
//   BFS distances in the complement           same walk, keep a dist array
//   is the complement connected               one component
//   is the complement bipartite               2-colour during the same BFS
//   "who is NOT friends with whom" grouping   the literal reading
//   complement is dense, original is sparse   this; if the ORIGINAL is dense,
//                                             just build the complement
//   independent set in G = clique in G'       different problem, still NP-hard
//   minimum clique cover of G                 = chromatic number of G' --
//                                             graph/graph_coloring.cpp
// ============================================================================
const int IN = 200'123;
vector<int> adj[IN];                     // REAL edges, each list SORTED
int compid[IN];

// components of the complement graph. Returns the number of components.
int comp_components(int n) {
    set<int> unvisited;
    for (int i = 0; i < n; i++) unvisited.insert(i), compid[i] = -1;
    int c = 0;
    while (!unvisited.empty()) {
        int s = *unvisited.begin();
        unvisited.erase(unvisited.begin());
        queue<int> q;
        q.push(s), compid[s] = c;
        while (q.size()) {
            int u = q.front(); q.pop();
            vector<int> take;            // collect first, erase after
            for (int v : unvisited)
                if (!binary_search(adj[u].begin(), adj[u].end(), v))
                    take.push_back(v);   // no real edge => complement edge
            for (int v : take) {
                unvisited.erase(v);      // erase NOW -- see the box
                compid[v] = c, q.push(v);
            }
        }
        c++;
    }
    return c;
}
