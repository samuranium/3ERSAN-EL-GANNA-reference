// ============================================================================
// CHROMATIC NUMBER -- fewest colours so no edge joins two equal colours
//                     inclusion-exclusion over subsets, O(2^n * log k) per k
// ----------------------------------------------------------------------------
// WHEN: n <= ~20. Graph colouring is NP-hard; there is no polynomial version.
//   If n is larger the problem is NOT asking for the chromatic number -- look
//   for a special structure instead (see SHORTCUTS).
//
// THE COUNT: let I(S) = number of INDEPENDENT subsets of S (empty set included).
//   The number of ordered k-tuples of independent sets whose union is all of V:
//       C_k = sum over S of (-1)^(|V \ S|) * I(S)^k
//   The graph is k-colourable iff C_k != 0. Answer = smallest such k.
//   Colour classes ARE independent sets, so this counts colourings directly.
//
//   I(S) itself is one line of DP. Take the lowest vertex v in S; an
//   independent subset either omits v, or takes v and then avoids all of N(v):
//       I(S) = I(S \ v) + I(S \ (N(v) U {v}))
//
// ############################################################################
// #  THE ARITHMETIC IS DELIBERATELY UNSIGNED
// #
// #  C_k is astronomically large. This computes it modulo 2^64 by letting
// #  unsigned long long wrap. A nonzero C_k could in principle wrap to exactly
// #  0 and make the graph look non-k-colourable -- the odds are ~2^-64 on any
// #  real input, and this is the standard trick, but know that it is a
// #  probabilistic answer and not an exact one.
// #  Want certainty? Run it under two different large prime moduli instead.
// ############################################################################
//
// PITFALLS:
//   adj[v] must NOT contain v itself. A self-loop makes the graph uncolourable
//     and this code will loop to n and return n.
//   n = 0 returns 0; an edgeless graph returns 1 (every vertex one colour).
//   Memory is 8 * 2^n bytes: n = 20 is 8 MB, n = 24 is 128 MB. That is the real
//     ceiling, not the time.
//   Vertices are 0-indexed here -- the rest of the graph folder is 1-indexed.
//
// ----------------------------------------------------------------------------
// SHORTCUTS -- check these before running anything
// ----------------------------------------------------------------------------
//   no edges                    -> 1
//   bipartite (BFS 2-colour)    -> 2       most "colour it" problems are this
//   odd cycle present           -> >= 3
//   tree / forest               -> 2
//   cycle graph                 -> 2 if even length, 3 if odd
//   complete graph K_n          -> n
//   planar                      -> <= 4 (four colour theorem), and <= 5 is easy
//   interval graph              -> = max clique, greedy by left endpoint
//   any graph                   -> <= maxdeg + 1, greedy achieves it
//                                  (Brooks: = maxdeg unless complete or odd cycle)
//
// RELATED SUBSET DP, same shape: minimum clique cover of G = chromatic number
//   of the COMPLEMENT of G. Maximum independent set = maximum clique of the
//   complement. If a statement is about cliques, complement it and come back.
// ============================================================================
const int MAXN = 20;
int nv, adj[MAXN];                   // adj[v] = bitmask of v's neighbours
unsigned long long I[1 << MAXN];     // I[S] = # independent subsets of S

int chromatic() {
    if (!nv) return 0;
    int full = (1 << nv) - 1;
    I[0] = 1;
    for (int S = 1; S <= full; S++) {
        int v = __builtin_ctz(S);
        I[S] = I[S ^ (1 << v)] + I[S & ~(adj[v] | (1 << v))];
    }
    for (int k = 1; k <= nv; k++) {
        unsigned long long tot = 0;
        for (int S = 0; S <= full; S++) {
            unsigned long long p = 1, base = I[S];
            for (int e = k; e; e >>= 1, base *= base)
                if (e & 1) p *= base;
            tot += (__builtin_popcount(full ^ S) & 1) ? 0ULL - p : p;
        }
        if (tot) return k;
    }
    return nv;
}
