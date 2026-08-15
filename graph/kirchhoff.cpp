// ============================================================================
// KIRCHHOFF / MATRIX-TREE -- count spanning trees, O(n^3)
// ----------------------------------------------------------------------------
// Build the LAPLACIAN  L = D - A   (D diagonal of degrees, A the adjacency
// matrix, multi-edges counted with multiplicity). Delete ANY one row and the
// SAME column. The determinant of what is left is the number of spanning trees.
//
// Every row and column of L sums to zero, so det L = 0 always -- deleting one
// row and column is not a trick, it is the only way to get a number out.
//
// ############################################################################
// #  COUNT THE DETERMINANT MOD p, NEVER IN DOUBLES
// #
// #  Cayley: the complete graph on n vertices has n^(n-2) spanning trees. At
// #  n = 20 that is 2.6e23 -- past both long long and the exact range of a
// #  double. A long-double determinant with round() looks right on tiny cases
// #  and silently drifts on real ones.
// #  Use graph det_mod from math/gauss.cpp. If the problem wants the exact
// #  integer and it is small, compute mod two different primes and CRT.
// ############################################################################
//
// PITFALLS:
//   SELF-LOOPS must be skipped entirely. A self-loop can never be in a
//     spanning tree, and adding it to the degree corrupts the count.
//   MULTI-EDGES do count: two parallel edges give two distinct spanning trees.
//     Increment both A[u][v] and the degrees each time.
//   A disconnected graph gives 0 automatically -- no special case needed.
//   n == 1: the answer is 1 (the empty tree). The minor is 0x0 and det_mod of
//     an empty matrix must return 1; guard it explicitly.
//   WHICH row/column you delete does not matter for undirected graphs -- all
//     cofactors of L are equal. For DIRECTED it very much does: the index you
//     delete IS the root.
//   Vertices are 0-indexed here.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   number of spanning trees                 count_spanning_trees()
//   ... of a WEIGHTED graph, meaning the     same function -- put the weight
//       sum over trees of the product of     in A[u][v] instead of 1. The
//       their edge weights                   determinant is multilinear, so
//                                            this falls out for free.
//   ... where each edge has a probability    same, weights = probabilities;
//       and you want expected connectivity   this is network reliability
//   number of ARBORESCENCES rooted at r      count_arborescences() -- Tutte
//   spanning trees CONTAINING a given edge   contract it (merge endpoints),
//                                            then count on the smaller graph
//   spanning trees AVOIDING a given edge     delete it, then count
//   Cayley's formula, n^(n-2)                the complete graph case; useful
//                                            as a self-check, see the test
//   number of labelled forests / trees with  Prufer sequences, not this
//     given degrees
//   MINIMUM spanning tree                    completely unrelated -- Kruskal,
//                                            graph/MST/
//
// NOT THIS: counting spanning trees of a graph with 1e5 vertices. O(n^3) caps
//   you around n = 500. Large n needs special structure (planar, series-
//   parallel, or a product graph with a known formula).
// ============================================================================
// needs det_mod from math/gauss.cpp, and mod/add/subt/mult from math/modular.cpp

// Undirected. edges may repeat; self-loops are ignored.
// Pass weights all 1 to count trees, or real weights to get the weighted sum.
int count_spanning_trees(int n, const vector<array<int, 3>> &edges) {
    if (n == 1) return 1;                       // empty tree
    vector<vector<int>> L(n, vector<int>(n, 0));
    for (auto &[u, v, w] : edges) {
        if (u == v) continue;                   // self-loop: never in a tree
        L[u][v] = subt(L[u][v], w);
        L[v][u] = subt(L[v][u], w);
        L[u][u] = add(L[u][u], w);
        L[v][v] = add(L[v][v], w);
    }
    vector<vector<int>> M(n - 1, vector<int>(n - 1));   // drop row/col n-1
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - 1; j++) M[i][j] = L[i][j];
    return det_mod(M);
}

// Directed. Counts spanning arborescences ORIENTED TOWARDS root -- every
// vertex has exactly one outgoing tree edge and all paths lead to root.
// For arborescences pointing AWAY from the root, reverse every edge first.
int count_arborescences(int n, int root, const vector<array<int, 3>> &edges) {
    if (n == 1) return 1;
    vector<vector<int>> L(n, vector<int>(n, 0));
    for (auto &[u, v, w] : edges) {             // edge u -> v
        if (u == v) continue;
        L[u][v] = subt(L[u][v], w);
        L[u][u] = add(L[u][u], w);              // OUT-degree on the diagonal
    }
    vector<vector<int>> M(n - 1, vector<int>(n - 1));   // drop row/col `root`
    for (int i = 0, ii = 0; i < n; i++) {
        if (i == root) continue;
        for (int j = 0, jj = 0; j < n; j++) {
            if (j == root) continue;
            M[ii][jj++] = L[i][j];
        }
        ii++;
    }
    return det_mod(M);
}
