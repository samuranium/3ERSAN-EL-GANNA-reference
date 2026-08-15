// ============================================================================
// VIRTUAL TREE (auxiliary tree) -- shrink n to O(k), O(k log k) per query
// ----------------------------------------------------------------------------
// WHEN: queries hand you k marked vertices of a huge tree, and SUM of k over
//   all queries is bounded (typically 2e5) while n is 2e5 too. Running an
//   O(n) DP per query is O(n*q) and dies. The virtual tree keeps only the
//   marked vertices plus the branch points between them -- O(k) nodes -- and
//   the original DP runs on that instead.
//
// ############################################################################
// #  THE NODE SET IS THE MARKS PLUS THE LCAs OF ADJACENT-IN-DFS-ORDER PAIRS
// #
// #  Sort the marks by tin. Then the ONLY extra vertices you need are
// #  lca(a[i], a[i+1]) for consecutive i. Not all pairwise LCAs -- that would
// #  be O(k^2) and is unnecessary: the deepest branch point between any two
// #  marks already appears between some adjacent pair.
// #
// #  This is the whole construction, and the reason it is O(k log k) rather
// #  than O(k^2 log k).
// ############################################################################
//
// ############################################################################
// #  EDGE WEIGHTS ARE DISTANCES, NOT 1
// #
// #  An edge of the virtual tree stands for a whole PATH in the original.
// #  Its weight is dist(parent, child) -- depth difference, or a path
// #  aggregate. Treating virtual edges as unit length is the standard bug and
// #  it produces answers that are plausible but too small.
// ############################################################################
//
// PITFALLS:
//   Clear only the O(k) nodes you touched, never the whole n-sized arrays.
//     Clearing 2e5 entries per query is exactly the O(n*q) you were avoiding.
//   The root of the virtual tree is the first element after sorting -- which
//     is the LCA of everything, not necessarily vertex 1.
//   Deduplicate after inserting the LCAs; a mark can also be a branch point.
//   Sorting by tin must happen twice: once before inserting LCAs, once after.
//   Needs tin/tout and lca() from DS/LCA/binary lifting.cpp.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS -- what you run on the small tree
// ----------------------------------------------------------------------------
//   min cost to disconnect k marked nodes from the root   tree DP on the VT,
//                                                         edge cost = min edge
//                                                         on the real path
//   sum of pairwise distances among the marks             classic VT DP
//   is there a vertex on every marked pair's path         VT diameter / centre
//   count marked nodes in each subtree                    VT subtree DP
//   Steiner tree of the marks (tree case)                 total VT edge weight
//   "delete these k nodes, what breaks"                   VT connectivity
//   any per-query tree DP with sum(k) bounded             the general case
//
// NOT THIS: if k is close to n, or there is only one query, just run the DP on
//   the real tree. The virtual tree only pays off when sum(k) << n*q.
// ============================================================================
// needs: tin[], dep[], lca(), and anc() from DS/LCA/binary lifting.cpp
const int VN = 200'123;
vector<pair<int, ll>> vt[VN];      // virtual tree: (child, real distance)
int vstk[VN], vtop;

// marks: the k vertices. Returns the ROOT of the virtual tree.
// vt[] is filled for exactly the nodes involved; clear them afterwards.
int build_virtual(vector<int> marks) {
    sort(marks.begin(), marks.end(),
         [](int a, int b) { return tin[a] < tin[b]; });
    int k = marks.size();
    for (int i = 0; i + 1 < k; i++)                  // adjacent pairs only
        marks.push_back(lca(marks[i], marks[i + 1]));
    sort(marks.begin(), marks.end(),
         [](int a, int b) { return tin[a] < tin[b]; });
    marks.erase(unique(marks.begin(), marks.end()), marks.end());
    for (int u : marks) vt[u].clear();               // clear ONLY these
    vtop = 0;
    for (int u : marks) {
        while (vtop && !anc(vstk[vtop - 1], u)) vtop--;
        if (vtop) {
            int p = vstk[vtop - 1];
            vt[p].push_back({u, (ll)dep[u] - dep[p]});   // REAL distance
        }
        vstk[vtop++] = u;
    }
    return marks[0];                                 // deepest common ancestor
}
