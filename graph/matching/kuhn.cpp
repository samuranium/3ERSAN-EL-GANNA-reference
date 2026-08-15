// ============================================================================
// KUHN -- maximum bipartite matching, O(V * E)
// ----------------------------------------------------------------------------
// Repeatedly look for an AUGMENTING PATH: an alternating path from a free left
// vertex to a free right vertex. Finding one raises the matching by exactly 1,
// and Berge's theorem says a matching is maximum exactly when none exists.
//
// O(V*E) is the worst case and it is essentially never reached with the greedy
// warm start below. In practice this handles n, m ~ 1e5 edges comfortably --
// which is why it is preferred to Hopcroft-Karp (O(E sqrt V)) despite the
// worse bound. Reach for Hopcroft only when Kuhn actually TLEs.
//
// ############################################################################
// #  THE GREEDY WARM START IS NOT OPTIONAL IN PRACTICE
// #
// #  Matching every left vertex to any free neighbour first typically fixes
// #  80-90% of the matching in O(E), leaving few augmentations to search for.
// #  Without it, adversarial orderings turn this into a genuine O(V*E) and it
// #  will TLE on tests that are otherwise easy.
// ############################################################################
//
// ############################################################################
// #  used[] IS PER-AUGMENTATION, NOT GLOBAL
// #
// #  It must be cleared before EVERY try_kuhn from a new left vertex. Clearing
// #  it once outside the loop finds one path and then reports a maximum
// #  matching that is far too small -- and it looks like a working algorithm.
// #  Clearing it INSIDE the recursion instead makes it non-terminating.
// ############################################################################
//
// PITFALLS:
//   Two separate index spaces. g[] is indexed by LEFT vertex and holds RIGHT
//     vertex ids; used[] and mr[] are indexed by RIGHT. Mixing them is the
//     other classic bug. Both sides are 0-indexed here.
//   try_kuhn recurses up to V deep. At V = 1e5 that is a stack risk.
//   The graph must be BIPARTITE. On a general graph you need Blossom, which is
//     a different algorithm entirely and far longer.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS -- the theorems that make this worth far more than "matching"
// ----------------------------------------------------------------------------
//   maximum matching                     kuhn()
//   MINIMUM VERTEX COVER                 == maximum matching (Konig). Recover
//                                        the actual set with min_vertex_cover()
//   MAXIMUM INDEPENDENT SET              == n - maximum matching, and it is
//                                        the complement of the vertex cover
//   MINIMUM PATH COVER of a DAG          == n - maximum matching on the split
//                                        graph (each vertex as both left and
//                                        right). Counts vertex-disjoint paths.
//   MAXIMUM ANTICHAIN in a poset         Dilworth, via the path cover above
//   assign tasks to workers              the literal reading
//   place rooks / dominoes on a board    colour the board like a chessboard;
//     with forbidden cells               cells become the two sides
//   pick one representative per group    Hall's theorem tells you when a
//                                        perfect matching exists: every subset
//                                        S of the left has |N(S)| >= |S|
//   maximum matching with WEIGHTS        NOT this -- Hungarian, or MCMF
//   matching in a GENERAL graph          NOT this -- Blossom
// ============================================================================
const int KN = 100005;
vector<int> g[KN];          // g[left] = list of RIGHT vertex ids
int ml[KN], mr[KN];         // ml[left] / mr[right], -1 when free
bool used[KN];

bool try_kuhn(int v) {
    for (int to : g[v]) {
        if (used[to]) continue;
        used[to] = true;
        if (mr[to] == -1 || try_kuhn(mr[to])) {
            mr[to] = v, ml[v] = to;
            return true;
        }
    }
    return false;
}
int kuhn(int nl, int nr) {
    fill(ml, ml + nl, -1), fill(mr, mr + nr, -1);
    int res = 0;
    for (int v = 0; v < nl; v++)          // greedy warm start -- see the box
        for (int to : g[v])
            if (mr[to] == -1) { mr[to] = v, ml[v] = to, res++; break; }
    for (int v = 0; v < nl; v++) {
        if (ml[v] != -1) continue;
        fill(used, used + nr, false);     // per augmentation, see the box
        if (try_kuhn(v)) res++;
    }
    return res;
}
// KONIG: from a maximum matching, a minimum vertex cover of the same size.
// Returns (left vertices chosen, right vertices chosen).
// Alternate from every FREE left vertex; the cover is (unvisited left) plus
// (visited right). Max independent set is exactly the complement.
pair<vector<int>, vector<int>> min_vertex_cover(int nl, int nr) {
    vector<char> visL(nl, 0), visR(nr, 0);
    function<void(int)> dfs = [&](int v) {
        visL[v] = 1;
        for (int to : g[v])
            if (!visR[to] && ml[v] != to) {
                visR[to] = 1;
                if (mr[to] != -1) dfs(mr[to]);
            }
    };
    for (int v = 0; v < nl; v++)
        if (ml[v] == -1 && !visL[v]) dfs(v);
    vector<int> L, R;
    for (int v = 0; v < nl; v++) if (!visL[v]) L.push_back(v);
    for (int u = 0; u < nr; u++) if (visR[u]) R.push_back(u);
    return {L, R};
}
