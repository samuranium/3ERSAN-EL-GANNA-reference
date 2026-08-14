// ============================================================================
// REROOTING -- the answer for EVERY root, in O(n) total
// ----------------------------------------------------------------------------
// TWO THINGS ARE CALLED "REROOTING". Do not mix them up:
//   (a) REROOTING DP -- this file. You can compute f(root) with one DFS; the
//       problem wants f(v) for all v. Naive is O(n^2), this is O(n).
//   (b) REROOTING AN LCA QUERY -- the tree is fixed, one query names a
//       different root. No DP, no rebuild, see [T4] in DS/LCA/binary lifting.cpp:
//       lca_rooted_at_r(u,v) = deepest of lca(u,v), lca(u,r), lca(v,r).
//
// THE TELL for (a): "for each vertex, output ..." on a tree. If the statement
//   asks per-vertex and a single-root version is easy, it is this.
//
// ----------------------------------------------------------------------------
// THE FRAMEWORK
// ----------------------------------------------------------------------------
//   down[u] = answer over u's SUBTREE      (tree rooted at 1)
//   up[u]   = answer over EVERYTHING ELSE, entering u through its parent
//   ans[u]  = combine(down[u], up[u])
//
//   Pass 1 (post-order):  down[u] from down[children]
//   Pass 2 (pre-order):   up[child] from up[u] and u's OTHER children
//
// PASS 2 IS THE ONLY HARD PART -- "u's other children" is the whole game:
//
//   INVERTIBLE merge (+, xor, count, product with no zeros):
//       take u's total and SUBTRACT the child's contribution. One line.
//
//   NON-INVERTIBLE merge (max, min, gcd, "best two"):
//       you cannot subtract. Two options --
//         prefix/suffix arrays over the child list: pre[i] = merge of children
//           0..i-1, suf[i] = merge of i+1..end, then exclude child i in O(1).
//           This is the GENERAL answer and always works.
//         or, for max/min specifically, keep the BEST TWO. If the best comes
//           through the child you are excluding, use the second best.
//           Shorter to type; see example B.
//
// ############################################################################
// #  THE PITFALL THAT COSTS THE MOST TIME
// #
// #  up[root] must be the IDENTITY OF YOUR MERGE. Not 0 by reflex.
// #      sum 0      count 0      xor 0      gcd 0
// #      max -INF   min +INF     product 1  best-two (0,0) or (-INF,-INF)
// #
// #  SYMPTOM: ans[root] comes out RIGHT and every other vertex is wrong. The
// #  instinct is to go hunting in pass 2's exclusion logic, which is the
// #  expensive place to look. Check this line first -- it is one character.
// ############################################################################
//
// OTHER PITFALLS:
//   Recursion: two DFS passes over n = 2e5. Same stack caveat as the rest of
//     the tree code here.
//   ll for anything summing distances: n * n = 4e10 at n = 2e5.
//   Multi-test: clear g[], down[], up[] for 1..n, not for 1..N.
//   The child loop in pass 2 must use up[u] as it was BEFORE you descend --
//     write up[v] first, then recurse. Do not recurse inside the accumulation.
//
// ============================================================================
// EXAMPLE A -- sum of distances from every node   (CSES Tree Distances II)
// ----------------------------------------------------------------------------
// Merge is +, so it is invertible and pass 2 collapses to a single shift.
// Move the root from u to a child v: every one of the sz[v] nodes inside v's
// subtree gets 1 CLOSER, every one of the other n - sz[v] gets 1 further.
//     ans[v] = ans[u] - sz[v] + (n - sz[v]) = ans[u] + n - 2*sz[v]
// That subtraction is only legal because + has an inverse. Keep the general
// framework in mind for when it does not -- see example B.
// ============================================================================
const int N = 200'123;
vector<int> g[N];
int n, sz[N];
ll down[N], ans[N];

void dfs1(int u, int p) {
    sz[u] = 1, down[u] = 0;
    for (int v : g[u])
        if (v != p) {
            dfs1(v, u);
            sz[u] += sz[v];
            down[u] += down[v] + sz[v];   // every node in v's subtree is 1 further
        }
}
void dfs2(int u, int p) {
    for (int v : g[u])
        if (v != p) {
            ans[v] = ans[u] + n - 2 * sz[v];
            dfs2(v, u);
        }
}
// run: dfs1(1, 0); ans[1] = down[1]; dfs2(1, 0);
//      ^ merge is +, so the root seed is down[1] itself (up[root] = 0 = identity)

// ============================================================================
// EXAMPLE B -- eccentricity: farthest node from each vertex
// ----------------------------------------------------------------------------
// Merge is max -- NOT invertible. Best-two form: d1 = deepest downward, d2 =
// second deepest through a DIFFERENT child. When excluding child v, if the best
// came through v then the best without v is d2.
//   ecc[u] = max(d1[u], updep[u])
// Sanity check: max over all u of ecc[u] is the diameter.
// ============================================================================
int d1[N], d2[N], updep[N];

void dfs1b(int u, int p) {
    d1[u] = d2[u] = 0;
    for (int v : g[u])
        if (v != p) {
            dfs1b(v, u);
            int c = d1[v] + 1;
            if (c > d1[u]) d2[u] = d1[u], d1[u] = c;
            else if (c > d2[u]) d2[u] = c;
        }
}
void dfs2b(int u, int p) {
    for (int v : g[u])
        if (v != p) {
            int best = (d1[v] + 1 == d1[u] ? d2[u] : d1[u]);  // u's best WITHOUT v
            updep[v] = max(updep[u], best) + 1;
            dfs2b(v, u);
        }
}
// run: dfs1b(1, 0); updep[1] = 0; dfs2b(1, 0);
//      ^ identity for "deepest upward" is 0 here because depths are >= 0. If
//        your values can go negative this must be -INF, not 0.

// ============================================================================
// THE GENERAL PASS 2, when neither subtraction nor best-two fits
// ----------------------------------------------------------------------------
//   vector<int> ch;                       // children of u, in order
//   vector<T> pre(k+1, ID), suf(k+1, ID); // ID = identity of merge
//   for (int i = 0; i < k; i++)     pre[i+1] = merge(pre[i], f(ch[i]));
//   for (int i = k-1; i >= 0; i--)  suf[i]   = merge(suf[i+1], f(ch[i]));
//   for (int i = 0; i < k; i++)
//       up[ch[i]] = lift(merge(up[u], merge(pre[i], suf[i+1])));
// where f(v) is v's downward contribution and lift() is the one-edge step.
// O(deg u) per node, O(n) overall. Use this whenever you are unsure -- it is
// never wrong, and the invertible case is just an optimisation of it.
//
// ----------------------------------------------------------------------------
// WHAT ELSE IS THIS SHAPE
// ----------------------------------------------------------------------------
//   sum / max distance from every node          examples A and B
//   # of paths through each vertex              sz[v] * (n - sz[v]) per edge
//   count subtrees / colourings containing v    merge = product, invertible
//   best answer if the tree is hung at v        any "for each root" statement
//   tree diameter                               max ecc, or two BFS (shorter)
//   sum over all (u,v) pairs of some path stat  often rerooting + contribution
//
// NOT REROOTING: a single fixed root with path queries -> HLD. Queries about
//   ancestors -> DS/LCA/binary lifting.cpp. Small-to-large / DSU on tree is a
//   different technique for "answer per subtree" WITHOUT the second pass.
// ============================================================================
