// ============================================================================
// LCA by BINARY LIFTING -- O(n log n) build, O(log n) query, O(1) is-ancestor
// ----------------------------------------------------------------------------
// up[k][u] = the 2^k-th ancestor of u.  Root points at itself, so lifting
// saturates at the root instead of running off the array -- that self-loop is
// what removes every bounds check below.
//
// tin/tout are Euler entry/exit stamps. They give is-ancestor in O(1), and the
// lca() here is the "climb while it is still NOT an ancestor" version, which
// needs no depth comparison and no swap.
//
// PITFALLS:
//   dfs() is recursive -- a path graph at n = 2e5 will blow the stack. Same
//     caveat as HLD/ and centroid/ in this folder.
//   Call as dfs(root, root). Passing -1 breaks up[0][root] and the saturation.
//   LG must satisfy 2^LG > n. LG = 18 covers 2e5, LG = 20 covers 1e6.
//   LCA DEPENDS ON THE ROOT. Re-rooting does not need a rebuild -- see [T4].
//   Memory is LG * N ints: 18 * 2e5 * 4 = 14 MB. Fine, but it is not free.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS -- all O(log n) unless noted
// ----------------------------------------------------------------------------
//   [1]  lca(u, v)                        lca()
//   [2]  is u an ancestor of v            anc()                       O(1)
//   [3]  dist(u, v) in edges              dist()
//   [4]  k-th ancestor of u               kth()
//   [5]  k-th node on the path u -> v     jump()
//   [6]  is w on the path u -> v          dist(u,w) + dist(w,v) == dist(u,v)
//   [7]  # nodes on the path u -> v       dist(u,v) + 1
//   [8]  max/min/sum edge on a path       second table, see [T1]
//   [9]  LCA of a whole SET S             see [T2]
//   [10] meeting point of three nodes     see [T3]
//   [11] LCA when the tree is re-rooted   see [T4]
//   [12] last ancestor satisfying P       see [T5]
//   [13] virtual / auxiliary tree         see [T6]
//
// ----------------------------------------------------------------------------
// TRICKS
// ----------------------------------------------------------------------------
// [T1] PATH AGGREGATE. Carry a second table alongside up[][], indexed the same
//      way, holding the aggregate of the 2^k edges you jump over:
//        mx[0][u] = weight(u, parent(u))
//        mx[k][u] = max(mx[k-1][u], mx[k-1][ up[k-1][u] ])
//      Then climb u and v to the LCA separately, folding as you go. Works for
//      max, min, sum, gcd -- anything associative. NOT for anything needing
//      order (like "first edge > x"); for that use [T5].
//      If you also need UPDATES, this is the wrong tool -> HLD/ in this folder.
//
// [T2] LCA OF A SET. lca(S) = lca(argmin tin, argmax tin) over S. Two lookups,
//      not |S|. This is the workhorse behind [T6].
//
// [T3] MEETING POINT OF a, b, c -- the unique node where the three paths join.
//      It is the DEEPEST of lca(a,b), lca(b,c), lca(a,c). (Two of the three are
//      always equal; the odd one out is the answer.)
//      Sum of the three pairwise distances / 2 = total edges in the Steiner tree.
//
// [T4] RE-ROOTING AT r WITHOUT REBUILDING. The LCA of u, v in the tree rooted
//      at r is the DEEPEST of lca(u,v), lca(u,r), lca(v,r) -- all measured in
//      the original rooting. Same shape as [T3], and it is why "queries with a
//      moving root" almost never needs a second DFS.
//
// [T5] BINARY SEARCH ON THE PATH. To find the highest / last ancestor where a
//      MONOTONE predicate P still holds, replace the test in lca()'s loop:
//        for (int k = LG-1; k >= 0; k--) if (P(up[k][u])) u = up[k][u];
//      Uses: last ancestor with value <= x, first ancestor of a given colour,
//      "how far up before the sum exceeds S" (pair it with [T1]).
//
// [T6] VIRTUAL (AUXILIARY) TREE -- when queries give you k marked nodes and
//      sum(k) is bounded, but n is huge. Build a tree of size O(k):
//        1. sort the marked nodes by tin
//        2. insert lca(consecutive pairs); sort + unique again
//        3. connect with a stack: pop while !anc(stack.top(), cur), then
//           parent(cur) = stack.top(), push cur
//      Then run the real DP on O(k) nodes instead of n. Edge weights in the
//      virtual tree are dist() in the original.
//
// [T7] BINARY LIFTING IS NOT ABOUT TREES. up[k][u] = "apply f 2^k times" works
//      for ANY functional graph -- next-greater pointers, successor links,
//      "where am I after k operations", doubling on a permutation. The tree
//      case is just f = parent. If your problem says "repeat k times, k <= 1e18",
//      this table is the answer.
//
// ----------------------------------------------------------------------------
// ALTERNATIVES -- pick deliberately
// ----------------------------------------------------------------------------
//   Euler tour + sparse table   O(1) query, O(n log n) build. Faster queries,
//                               more code, and no k-th ancestor for free.
//   HLD                         you already have it; gives LCA plus path
//                               queries WITH updates. Use it when you need both.
//   Tarjan offline (DSU)        O(n a(n)) total, but all queries must be known
//                               up front. Rarely worth it in a contest.
//   Tree changes shape          binary lifting cannot handle link/cut -> LCT.
// ============================================================================
const int N = 200'123, LG = 18;
vector<int> g[N];
int up[LG][N], dep[N], tin[N], tout[N], tt;

void dfs(int u, int p) {            // call dfs(root, root)
    tin[u] = tt++;
    up[0][u] = p;
    for (int k = 1; k < LG; k++) up[k][u] = up[k - 1][up[k - 1][u]];
    for (int v : g[u])
        if (v != p) dep[v] = dep[u] + 1, dfs(v, u);
    tout[u] = tt++;
}
// u is an ancestor of v (a node is its own ancestor)
bool anc(int u, int v) { return tin[u] <= tin[v] && tout[v] <= tout[u]; }

int lca(int u, int v) {
    if (anc(u, v)) return u;
    if (anc(v, u)) return v;
    for (int k = LG - 1; k >= 0; k--)
        if (!anc(up[k][u], v)) u = up[k][u];
    return up[0][u];
}
int kth(int u, int k) {             // k-th ancestor, -1 if it is above the root
    if (k > dep[u]) return -1;
    for (int i = 0; k; k >>= 1, i++)
        if (k & 1) u = up[i][u];
    return u;
}
int dist(int u, int v) { return dep[u] + dep[v] - 2 * dep[lca(u, v)]; }

int jump(int u, int v, int k) {     // k-th node on the path u -> v, k = 0 is u
    int l = lca(u, v), du = dep[u] - dep[l];
    return k <= du ? kth(u, k) : kth(v, dep[u] + dep[v] - 2 * dep[l] - k);
}
