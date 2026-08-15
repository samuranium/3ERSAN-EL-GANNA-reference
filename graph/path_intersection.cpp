// ============================================================================
// PATH INTERSECTION ON A TREE -- where do two paths overlap, O(log n)
// ----------------------------------------------------------------------------
// The intersection of two paths in a tree is always EITHER EMPTY OR A PATH --
// never two pieces. That is the fact the whole routine rests on: it means the
// answer is fully described by two endpoints, and those endpoints must be
// among a small fixed set of LCAs.
//
// ############################################################################
// #  THE CANDIDATE SET IS SIX NODES, AND THAT IS PROVABLE
// #
// #      lca(a,b)  lca(c,d)  lca(a,c)  lca(a,d)  lca(b,c)  lca(b,d)
// #
// #  Any endpoint of the overlap is a branch point of the two paths, and every
// #  branch point of two paths is one of those six. So: keep the ones lying on
// #  BOTH paths, and the answer is the farthest-apart surviving pair.
// #
// #  Trying to reason case-by-case about which LCA is "the" answer is where
// #  this goes wrong -- there are more cases than people expect. Filter and
// #  take the max distance instead.
// ############################################################################
//
// on_path(x, a, b) is the workhorse:  dist(a,x) + dist(x,b) == dist(a,b).
// One identity, no case analysis, and it is why this file is short.
//
// PITFALLS:
//   The intersection can be a SINGLE VERTEX (u == v). That is not "empty" --
//     distinguish them, since "do the paths share a vertex" and "do they share
//     an edge" are different questions and both get asked.
//   Needs lca(), dep[] from DS/LCA/binary lifting.cpp.
//   For EDGE intersection rather than vertex, the answer is the same path but
//     its length in edges is dist(u,v); a single shared vertex means 0 edges.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   do two paths share a vertex             intersect() returns non-empty
//   do two paths share an edge              non-empty AND u != v
//   how many vertices do they share         dist(u,v) + 1
//   how many edges                          dist(u,v)
//   does path 1 contain path 2 entirely     intersection == path 2
//   is vertex x on the path a..b            on_path()
//   is edge (p,q) on the path a..b          on_path for both, and adjacent
//   q queries "do these two paths meet"     this, O(log n) each
//   pairwise-intersecting family of paths   Helly property on trees: if every
//                                           PAIR intersects, all of them share
//                                           a common vertex -- so check pairs,
//                                           or intersect them one by one
//   add 1 to every vertex on many paths     different problem -- difference on
//                                           the tree, DS/2d partial sum.cpp
//                                           style, or HLD
// ============================================================================
// needs lca(), dep[] from DS/LCA/binary lifting.cpp
int distv(int u, int v) { return dep[u] + dep[v] - 2 * dep[lca(u, v)]; }
bool on_path(int x, int a, int b) {
    return distv(a, x) + distv(x, b) == distv(a, b);
}
// intersection of path(a,b) and path(c,d).
// returns {-1,-1} if empty; {u,u} if it is a single vertex; else the endpoints.
pair<int, int> intersect(int a, int b, int c, int d) {
    int cand[6] = {lca(a, b), lca(c, d), lca(a, c), lca(a, d), lca(b, c), lca(b, d)};
    int u = -1, v = -1, best = -1;
    for (int i = 0; i < 6; i++) {
        if (!on_path(cand[i], a, b) || !on_path(cand[i], c, d)) continue;
        for (int j = i; j < 6; j++) {
            if (!on_path(cand[j], a, b) || !on_path(cand[j], c, d)) continue;
            int t = distv(cand[i], cand[j]);
            if (t > best) best = t, u = cand[i], v = cand[j];
        }
    }
    return best < 0 ? make_pair(-1, -1) : make_pair(u, v);
}
