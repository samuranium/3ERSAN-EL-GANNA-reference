// ============================================================================
// SECOND-BEST MST -- O(E log E + E log V)
// ----------------------------------------------------------------------------
// THE IDEA: the second-best spanning tree differs from the MST by EXACTLY ONE
//   edge. So: build the MST, then for every non-tree edge (u,v,w), add it --
//   that closes a cycle -- and delete the heaviest edge on the tree path u..v.
//   Best over all non-tree edges is the answer.
//       cost = mst - maxEdgeOnPath(u,v) + w
//
// ############################################################################
// #  WHY TWO MAXIMA AND NOT ONE
// #
// #  If the heaviest edge on the path EQUALS w, swapping it changes nothing --
// #  you get the same total, i.e. the same-weight tree, not a second-best one.
// #  So when max == w you must remove the largest edge STRICTLY SMALLER than w,
// #  which is why m2[] tracks the second largest DISTINCT value.
// #
// #  Skipping this is the standard bug. It only shows on graphs with repeated
// #  weights, and it returns the MST cost itself as the "second best".
// ############################################################################
//
// PITFALLS:
//   -1 means "no edge" throughout; weights are assumed >= 0. If your weights
//     can be negative, move the sentinel to LLONG_MIN.
//   A non-tree edge whose path max equals w AND has no smaller second max
//     yields no candidate -- skip it, do not treat -1 as a real edge.
//   Disconnected input has no MST at all; check the DSU merge count first.
//   Needs a strictly-second-best DISTINCT tree. If the problem allows a tree of
//     equal weight, drop the m2 logic and just use m1.
//
// RELATED:
//   The path-maximum machinery here is [T1] in DS/LCA/binary lifting.cpp,
//   specialised to (max, second max). Same table, richer payload.
//   MINIMAX / bottleneck path between two nodes = the path max on the MST --
//   that is this same query with no second-best step.
//   Kruskal itself: graph/MST/kruskal with dsu.cpp
// ============================================================================
const int N = 100'123, LG = 17;
struct Ed { int u, v, w; };
vector<Ed> es;
int dsu[N];
vector<pair<int, int>> g[N];          // MST adjacency (to, w)
int up[LG][N], dep[N], m1[LG][N], m2[LG][N];

int find(int x) { return dsu[x] == x ? x : dsu[x] = find(dsu[x]); }

// fold one weight into the running (largest, second largest DISTINCT) pair
void upd(int &a, int &b, int x) {
    if (x > a) b = a, a = x;
    else if (x < a && x > b) b = x;
}
void dfs(int u, int p, int w) {
    up[0][u] = p, m1[0][u] = w, m2[0][u] = -1;
    for (int k = 1; k < LG; k++) {
        int mid = up[k - 1][u];
        up[k][u] = up[k - 1][mid];
        m1[k][u] = m1[k - 1][u], m2[k][u] = m2[k - 1][u];
        upd(m1[k][u], m2[k][u], m1[k - 1][mid]);
        upd(m1[k][u], m2[k][u], m2[k - 1][mid]);
    }
    for (auto &[v, ww] : g[u])
        if (v != p) dep[v] = dep[u] + 1, dfs(v, u, ww);
}
// (largest, second largest distinct) edge weight on the tree path u..v
pair<int, int> path_max(int u, int v) {
    int a = -1, b = -1;
    if (dep[u] < dep[v]) swap(u, v);
    for (int k = 0, d = dep[u] - dep[v]; k < LG; k++)
        if (d >> k & 1)
            upd(a, b, m1[k][u]), upd(a, b, m2[k][u]), u = up[k][u];
    if (u == v) return {a, b};
    for (int k = LG - 1; k >= 0; k--)
        if (up[k][u] != up[k][v]) {
            upd(a, b, m1[k][u]), upd(a, b, m2[k][u]);
            upd(a, b, m1[k][v]), upd(a, b, m2[k][v]);
            u = up[k][u], v = up[k][v];
        }
    upd(a, b, m1[0][u]), upd(a, b, m2[0][u]);
    upd(a, b, m1[0][v]), upd(a, b, m2[0][v]);
    return {a, b};
}
// returns {mst, second best}; second is LLONG_MAX if none exists
pair<ll, ll> second_best(int n) {
    sort(es.begin(), es.end(), [](const Ed &x, const Ed &y) { return x.w < y.w; });
    for (int i = 1; i <= n; i++) dsu[i] = i, g[i].clear();
    vector<char> used(es.size(), 0);
    ll mst = 0; int taken = 0;
    for (int i = 0; i < (int)es.size(); i++) {
        auto [u, v, w] = es[i];
        if (find(u) == find(v)) continue;
        dsu[find(u)] = find(v), used[i] = 1, mst += w, taken++;
        g[u].push_back({v, w}), g[v].push_back({u, w});
    }
    if (taken != n - 1) return {-1, -1};          // disconnected: no MST
    dep[1] = 0, dfs(1, 1, -1);
    ll best = LLONG_MAX;
    for (int i = 0; i < (int)es.size(); i++) {
        if (used[i]) continue;
        auto [u, v, w] = es[i];
        auto [a, b] = path_max(u, v);
        int rem = (a != w ? a : b);               // see the box
        if (rem < 0) continue;
        best = min(best, mst - rem + w);
    }
    return {mst, best};
}
