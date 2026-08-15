// ============================================================================
// HOPCROFT-KARP -- maximum bipartite matching, O(E sqrt(V))
// ----------------------------------------------------------------------------
// Kuhn augments along ONE path per phase. Hopcroft-Karp finds a maximal set of
// SHORTEST vertex-disjoint augmenting paths per phase, and there are only
// O(sqrt V) phases. Each phase is a BFS (to layer the graph by distance) plus
// a DFS (to pull out disjoint paths through those layers).
//
// ############################################################################
// #  USE KUHN UNLESS IT ACTUALLY TLEs
// #
// #  graph/matching/kuhn.cpp is O(V*E) worst case but with the greedy warm
// #  start it beats this on nearly every real test, and it is a third of the
// #  code. Hopcroft-Karp earns its place only when E is large AND the graph is
// #  adversarial -- dense bipartite graphs around 1e5 vertices and 1e6 edges.
// #
// #  Everything Kuhn gives you (Konig vertex cover, independent set, path
// #  cover, Dilworth) applies here unchanged -- it is the same matching.
// ############################################################################
//
// ############################################################################
// #  THE LAYERING IS THE ALGORITHM
// #
// #  bfs() computes dist[] over FREE left vertices only, and dfs() may step
// #  from u to the next left vertex w only when dist[w] == dist[u] + 1. That
// #  restriction is what makes the extracted paths shortest AND disjoint.
// #  Drop it and you have written a slower Kuhn.
// #
// #  dist[] on the NIL sentinel is how "did any augmenting path exist" is
// #  answered -- if it stays INF, the matching is maximum and we stop.
// ############################################################################
//
// PITFALLS:
//   Both sides are 1-INDEXED here and 0 is the NIL sentinel. Mixing in a
//     0-indexed left vertex silently matches everything to NIL.
//   ml[] is indexed by LEFT, mr[] by RIGHT, and g[] holds RIGHT ids. Same
//     trap as Kuhn.
//   Clear ml/mr/dist per test case, not just g[].
//   The DFS is recursive with depth up to V.
//
// SUBPROBLEMS: identical to graph/matching/kuhn.cpp -- minimum vertex cover
//   (Konig), maximum independent set, minimum path cover of a DAG, Dilworth
//   antichains, Hall's condition. Only the running time differs.
// ============================================================================
const int HK = 100005;
const int HKINF = 1e9;
vector<int> hg[HK];               // hg[left] = RIGHT ids; both 1-indexed
int hml[HK], hmr[HK], hdist[HK];  // hml[left], hmr[right]
int hnl, hnr;

bool hk_bfs() {
    queue<int> q;
    for (int u = 1; u <= hnl; u++) {
        if (!hml[u]) hdist[u] = 0, q.push(u);
        else hdist[u] = HKINF;
    }
    hdist[0] = HKINF;                       // NIL
    while (q.size()) {
        int u = q.front(); q.pop();
        if (hdist[u] >= hdist[0]) continue;
        for (int v : hg[u]) {
            int w = hmr[v];                 // the left vertex currently on v
            if (hdist[w] == HKINF) hdist[w] = hdist[u] + 1, q.push(w);
        }
    }
    return hdist[0] != HKINF;               // was any augmenting path found
}
bool hk_dfs(int u) {
    if (!u) return true;                    // reached NIL: path complete
    for (int v : hg[u]) {
        int w = hmr[v];
        if (hdist[w] == hdist[u] + 1 && hk_dfs(w)) {   // layered step only
            hmr[v] = u, hml[u] = v;
            return true;
        }
    }
    hdist[u] = HKINF;                       // dead end: never revisit this phase
    return false;
}
int hopcroft_karp(int nl, int nr) {
    hnl = nl, hnr = nr;
    for (int i = 0; i <= nl; i++) hml[i] = 0;
    for (int i = 0; i <= nr; i++) hmr[i] = 0;
    int res = 0;
    while (hk_bfs())
        for (int u = 1; u <= nl; u++)
            if (!hml[u] && hk_dfs(u)) res++;
    return res;
}
