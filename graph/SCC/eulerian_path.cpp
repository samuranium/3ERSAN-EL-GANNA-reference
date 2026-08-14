// ============================================================================
// EULERIAN PATH / CIRCUIT -- Hierholzer, O(V + E), directed and undirected
// ----------------------------------------------------------------------------
// Use EVERY EDGE exactly once. (Every VERTEX once is Hamiltonian and NP-hard --
// the two get confused constantly and they are not related problems.)
//
// ----------------------------------------------------------------------------
// EXISTENCE -- check this before calling euler(), it does not check for you
// ----------------------------------------------------------------------------
//   UNDIRECTED    circuit : every degree even
//                 path    : exactly 2 odd-degree vertices (start/end at them)
//   DIRECTED      circuit : in == out for every vertex
//                 path    : one vertex with out-in == 1 (START there), one with
//                           in-out == 1 (the end), all others balanced
//
//   PLUS CONNECTIVITY, both cases: every vertex WITH AT LEAST ONE EDGE must lie
//   in one component. Isolated vertices are irrelevant and must be ignored --
//   testing "is the whole graph connected" is the standard wrong check.
//
// ############################################################################
// #  THE CHEAP UNIVERSAL CHECK
// #
// #      euler(s, m).size() == m + 1
// #
// #  Hierholzer walks until it is stuck. If the result is shorter than m+1,
// #  some edges were unreachable -- i.e. the graph was disconnected. So you get
// #  the connectivity half of the condition for free, and only have to test
// #  degrees yourself. Always assert this; a partial walk looks like a real
// #  answer otherwise.
// ############################################################################
//
// PITFALLS:
//   ptr[] is the current-arc pointer and MUST persist across visits to u. Reset
//     it per vertex and the algorithm degrades to O(E^2).
//   Undirected: push the edge into BOTH lists with the SAME id, and mark
//     used[id]. Without the shared id you will walk an edge twice.
//   Start vertex matters for a PATH: begin at an odd-degree vertex (undirected)
//     or the out-in == 1 vertex (directed). Starting anywhere else finds a
//     shorter walk and silently fails the size check.
//   For a CIRCUIT start at any vertex that HAS an edge, never at vertex 1 by
//     reflex -- vertex 1 may be isolated.
//   The result is reversed at the end; do not drop that.
//   Self-loops and multi-edges are fine and need no special casing.
//   Iterative on purpose -- recursive Hierholzer dies at E = 2e5.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
// [1] EDGE SEQUENCE instead of vertices: push the edge id alongside the vertex
//     when you descend, and collect ids in the same place you collect vertices.
//     Needed whenever edges carry data (words, dominoes, directions).
//
// [2] WORD / DOMINO CHAINS -- the classic disguise. Each word is an EDGE from
//     its first letter to its last letter, over 26 vertices. "Can the words be
//     chained" is exactly "does an Eulerian path exist". Do not model words as
//     vertices; that gives you Hamiltonian and an NP-hard problem.
//
// [3] DE BRUIJN SEQUENCE of order k over an alphabet of size a: vertices are
//     the a^(k-1) strings of length k-1, edges are the a^k strings of length k
//     (drop first char -> drop last char). Any Eulerian circuit spells it.
//
// [4] MAKE IT EULERIAN (Chinese postman / route inspection): pair up the
//     odd-degree vertices with minimum total shortest-path cost and duplicate
//     those paths. With few odd vertices this is a bitmask DP over pairings;
//     in general it is min-cost matching.
//
// [5] DIRECTED, BUT UNBALANCED: add a single artificial edge from the end
//     vertex back to the start vertex, find a CIRCUIT, then delete that edge
//     from the cycle. Turns the path case into the circuit case.
//
// [6] COUNTING Eulerian circuits: BEST theorem for directed graphs (number of
//     arborescences times a product of factorials). Rare; know the name.
//
// [7] SPLITTING INTO CYCLES: any graph with all degrees even decomposes into
//     edge-disjoint cycles -- that is exactly what Hierholzer's stack builds.
// ============================================================================
const int N = 200'123, M = 400'123;
vector<pair<int, int>> g[N];    // (to, edge id); undirected: both ends, SAME id
int ptr[N], din[N], dout[N];
bool used[M];

// Hierholzer, iterative. Returns the vertex sequence.
// VALID ONLY IF the returned size == m + 1 (see the box).
vector<int> euler(int s, int m) {
    vector<int> st{s}, res;
    while (st.size()) {
        int u = st.back();
        while (ptr[u] < (int)g[u].size() && used[g[u][ptr[u]].second]) ptr[u]++;
        if (ptr[u] == (int)g[u].size()) res.push_back(u), st.pop_back();
        else {
            auto [v, id] = g[u][ptr[u]++];
            used[id] = 1, st.push_back(v);
        }
    }
    reverse(res.begin(), res.end());
    return res;
}

// -1 if no Eulerian path/circuit can exist by DEGREES alone; else the vertex to
// start from. Connectivity is left to the size check on euler()'s result.
int start_undirected(int n) {
    int odd = -1, cnt = 0, any = -1;
    for (int i = 1; i <= n; i++) {
        if (g[i].size()) any = i;
        if (g[i].size() & 1) cnt++, odd = i;
    }
    if (cnt != 0 && cnt != 2) return -1;
    return cnt == 2 ? odd : any;          // path starts odd, circuit starts anywhere
}
int start_directed(int n) {
    int s = -1, e = -1, any = -1;
    for (int i = 1; i <= n; i++) {
        if (dout[i]) any = i;
        int df = dout[i] - din[i];
        if (df == 1) { if (s != -1) return -1; s = i; }
        else if (df == -1) { if (e != -1) return -1; e = i; }
        else if (df) return -1;
    }
    if ((s == -1) != (e == -1)) return -1;   // one surplus without the other
    return s != -1 ? s : any;
}
