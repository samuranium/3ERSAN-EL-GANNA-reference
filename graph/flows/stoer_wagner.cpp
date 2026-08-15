// ============================================================================
// STOER-WAGNER -- GLOBAL minimum cut, no s and t given, O(V^3)
// ----------------------------------------------------------------------------
// "Split the graph into two non-empty parts, minimising the total weight of
// the cut edges." Undirected, non-negative weights. No terminals are chosen --
// that is what makes this different from a max-flow min-cut, which needs an
// s and a t.
//
// The phase: grow a set A one vertex at a time, always adding the vertex most
// tightly connected to A (max total weight to A). The LAST two vertices added,
// call them s then t, satisfy:
//
//     the cut isolating t is a minimum s-t cut
//
// Record it, MERGE s and t into one vertex, and repeat. n-1 phases, each O(V^2)
// with a plain scan. Every s-t pair is eventually separated in some phase, so
// the best recorded cut is the global minimum.
//
// ############################################################################
// #  IT IS THE LAST TWO VERTICES OF THE PHASE, IN ORDER
// #
// #  `t` is the last vertex added, `s` the one before it. The candidate cut
// #  value is w[t] -- the total weight from t to everything else at that
// #  moment -- and the merge is s <- s + t.
// #
// #  Taking the first two, or merging the wrong pair, still terminates and
// #  still returns a cut. It is just not the minimum, on graphs where the
// #  answer is not also achieved by an isolated vertex.
// ############################################################################
//
// ############################################################################
// #  min cut <= min DEGREE, ALWAYS
// #
// #  Isolating the lowest-degree vertex is a valid cut, so the answer never
// #  exceeds the minimum weighted degree. Use it as a sanity bound on your
// #  output -- if the algorithm returns something larger, it is wrong.
// #  A DISCONNECTED graph has min cut 0.
// ############################################################################
//
// PITFALLS:
//   Weights must be NON-NEGATIVE. Negative weights break the greedy entirely.
//   n = 1 has no cut; return INF or handle it before calling.
//   Parallel edges: add their weights into one adjacency entry.
//   The adjacency matrix is destroyed by the merges -- copy it if you need it.
//   To RECOVER the two sides, track which original vertices have been merged
//     into each super-vertex (a vector<int> per node) and record the group of
//     `t` when you record a new best.
//   O(V^3) with a scan; a Fibonacci heap gives O(VE + V^2 log V) and is never
//     worth it at contest sizes.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   global min cut, undirected                    min_cut()
//   "minimum edges to disconnect the graph"        unit weights
//   edge connectivity of a graph                   = global min cut with
//                                                  unit weights
//   "is the graph k-edge-connected"                min cut >= k
//   min cut between a SPECIFIC s and t             max flow instead --
//                                                 graph/flows/Dinics.cpp
//   min cut between ALL pairs                      Gomory-Hu tree: n-1 max
//                                                 flows, then the min edge on
//                                                 the tree path
//   VERTEX connectivity                            not this -- min vertex cut
//                                                 needs split-vertex max flow
//                                                 over O(n^2) pairs
//   which vertices are on each side                track the merged groups,
//                                                 see the pitfall
//   directed global min cut                        not this either -- run max
//                                                 flow from a fixed s to every
//                                                 t and back
//
// NOT THIS:
//   s and t are given            -> max flow. Faster and simpler.
//   the graph is directed        -> Stoer-Wagner is undirected only.
//   you need a min VERTEX cut     -> different problem entirely.
// ============================================================================
const ll INF = 1e18 / 4;

int n;
ll g[505][505];                 // adjacency matrix, symmetric, non-negative

// global minimum cut. Destroys g[][].
ll min_cut() {
    if (n < 2) return INF;
    vector<int> v(n);
    for (int i = 0; i < n; i++) v[i] = i;
    ll best = INF;
    int m = n;
    while (m > 1) {
        vector<ll> w(m, 0);
        vector<char> inA(m, 0);
        int s = -1, t = -1;
        for (int i = 0; i < m; i++) {
            // pick the vertex most tightly connected to A
            int sel = -1;
            for (int j = 0; j < m; j++)
                if (!inA[j] && (sel < 0 || w[j] > w[sel])) sel = j;
            inA[sel] = 1;
            s = t, t = sel;                         // remember the LAST TWO
            if (i == m - 1) {
                best = min(best, w[t]);             // the cut isolating t
                if (s >= 0)
                    for (int j = 0; j < m; j++)     // merge t into s
                        g[v[s]][v[j]] += g[v[t]][v[j]],
                        g[v[j]][v[s]] = g[v[s]][v[j]];
                v.erase(v.begin() + t);
                m--;
                break;
            }
            for (int j = 0; j < m; j++)
                if (!inA[j]) w[j] += g[v[sel]][v[j]];
        }
    }
    return best;
}
