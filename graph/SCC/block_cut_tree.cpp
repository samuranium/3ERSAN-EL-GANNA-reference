// ============================================================================
// BLOCK-CUT TREE -- biconnected components + articulation points, O(V + E)
// ----------------------------------------------------------------------------
// WHAT IT IS
//   A BLOCK (biconnected component) is a maximal set of EDGES in which any two
//   edges lie on a common simple cycle. Equivalently: a maximal piece with no
//   articulation point strictly inside it. Blocks are sets of EDGES, not
//   vertices -- that is the detail everything else follows from, because one
//   vertex can sit in several blocks.
//
//   The tree has TWO KINDS OF NODE:
//       1..n            the original vertices
//       n+1 .. n+nb     one node per block
//   and a block node is joined to every vertex it contains. A vertex that is
//   NOT an articulation point ends up with degree 1 (it lives in one block); an
//   articulation point ends up with degree >= 2. That is what makes it a cut
//   vertex, stated structurally.
//
//   The result is a FOREST -- one tree per connected component.
//
//   a---b   d---e        blocks: {ab,bc,ca} and {cd,de,ec}
//    \ /     \ /         articulation: c
//     c-------c          tree:  [B1]--c--[B2]   (a,b hang off B1; d,e off B2)
//   (same vertex c)
//
// ----------------------------------------------------------------------------
// WHY -- and how it differs from the bridge tree you already have
// ----------------------------------------------------------------------------
//   graph/SCC/tarjan - bridges.cpp gives the BRIDGE TREE: cut on bridges, nodes
//   are 2-EDGE-connected components. That answers questions about deleting an
//   EDGE. This file answers questions about deleting a VERTEX. They are not
//   interchangeable: the picture above has NO bridges at all, yet deleting c
//   splits the graph. Zero bridges, one articulation point.
//
//       bridge tree   -> "does removing this EDGE disconnect u and v"
//       block-cut tree-> "does removing this VERTEX disconnect u and v"
//
// ----------------------------------------------------------------------------
// WHEN -- the phrasings that mean "build this"
// ----------------------------------------------------------------------------
//   "is there a path u -> v avoiding vertex w"
//   "which single vertex's removal disconnects u from v"
//   "count vertices lying on EVERY path from u to v"
//   "count vertices lying on SOME path from u to v"
//   "after deleting w, how many components / how big are they"
//   anything on a cactus, where every edge is in at most one cycle
//   two vertex-disjoint paths exist between u,v  <=>  they share a block
//
//   NOT this: edge deletion (bridge tree), directed graphs (SCC -- biconnec-
//   tivity is undirected only), or global vertex connectivity k >= 3 (that is
//   max-flow, graph/flows/).
//
// ----------------------------------------------------------------------------
// HOW TO USE IT ONCE BUILT
// ----------------------------------------------------------------------------
//   [1] DOES w SEPARATE u AND v?
//       Root the tree. w separates them iff w is an articulation point and it
//       lies on the tree path between u and v. With DS/LCA/binary lifting.cpp
//       that is O(log n) per query: w is on the path iff
//           (anc(w,u) || anc(w,v)) && anc(lca(u,v), w)
//       Handle w == u or w == v as the problem dictates.
//
//   [2] VERTICES ON EVERY u->v PATH = the articulation points on the tree path,
//       plus u and v themselves. Prefix sums over the tree give O(log n).
//
//   [3] VERTICES ON SOME u->v PATH = every ORIGINAL vertex contained in the
//       blocks along the tree path. Weight each block node by its vertex count
//       (careful: articulation points are shared, subtract the overlaps) and
//       sum along the path.
//
//   [4] COMPONENT SIZES AFTER DELETING w: the subtrees hanging off w in the
//       block-cut tree. deg(w) in the tree is the number of pieces w splits its
//       component into.
//
//   [5] TWO VERTEX-DISJOINT PATHS between u and v exist iff u and v are in a
//       common block (Menger, k = 2). One tree-adjacency test.
//
// PITFALLS:
//   Node ids: vertices are 1..n, blocks are n+1..n+nb. Size arrays for n + nb,
//     and nb can be as large as n-1 (a path graph is n-1 blocks).
//   A bridge is a block of exactly ONE edge. Isolated vertices form no block at
//     all -- guard if the problem counts them.
//   Root special case: the root is an articulation point iff it has >= 2 DFS
//     children. Handled below by counting children, not by low-link.
//   dfs is recursive; same caveat as the rest of the tree code here.
//   Multi-test: clear g, tin, low, st, bct, and reset timer and nb.
// ============================================================================
const int N = 200'123;
vector<int> g[N];               // original graph, 1..n
vector<int> bct[2 * N];         // block-cut forest: 1..n vertices, n+1.. blocks
int tin[N], low[N], tt, nb;     // nb = number of blocks so far
bool is_art[N];
stack<int> st;                  // vertex stack

void dfs(int u, int p) {
    tin[u] = low[u] = ++tt;
    st.push(u);
    int children = 0;
    for (int v : g[u]) {
        if (v == p) continue;
        if (tin[v]) { low[u] = min(low[u], tin[v]); continue; }
        children++;
        dfs(v, u);
        low[u] = min(low[u], low[v]);
        if (low[v] >= tin[u]) {         // u closes a block
            if (p != -1) is_art[u] = 1;
            int b = ++nb + N;           // this block's node id
            for (int x = 0; x != v; ) { // pop down to and including v
                x = st.top(); st.pop();
                bct[b].push_back(x), bct[x].push_back(b);
            }
            bct[b].push_back(u), bct[u].push_back(b);   // u belongs too, stays on the stack
        }
    }
    if (p == -1 && children > 1) is_art[u] = 1;         // root rule
}
// run: for (i = 1..n) if (!tin[i]) dfs(i, -1);
// block nodes are N+1 .. N+nb   (offset by N so ids never collide with vertices)
