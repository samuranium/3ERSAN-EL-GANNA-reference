// ============================================================================
// LGV / BEST / RYSER / HOOK LENGTH / FUSS-CATALAN -- the determinant-and-
// formula counting family
// ----------------------------------------------------------------------------
// Five results that each turn a hard count into one determinant or one product.
// They share nothing but that shape, and each is unrecognisable until you have
// seen it once.
//
//   LGV          non-intersecting path systems      = det of single-path counts
//   BEST         Eulerian circuits in a digraph     = arborescences * prod(d-1)!
//   RYSER        permanent / "one per row and col"  = 2^n inclusion-exclusion
//   HOOK LENGTH  standard Young tableaux            = n! / prod(hooks)
//   FUSS-CATALAN k-ary trees, paths above a slope   = C(kn+n, n)/(kn+1)
//
// ############################################################################
// #  LGV RETURNS A SIGNED SUM, NOT A COUNT
// #
// #  det(M) with M[i][j] = "paths from start i to end j" equals
// #      sum over permutations sigma of  sign(sigma) * (path systems realising
// #      sigma with no two paths crossing)
// #
// #  That equals the count of non-intersecting systems ONLY when the identity
// #  permutation is the sole one admitting a non-crossing connection. In a
// #  planar / "sources and sinks in compatible order" setup that holds. If it
// #  does not, the determinant is a signed sum and is not a count at all --
// #  it can even be negative.
// ############################################################################
//
// ############################################################################
// #  BEST COUNTS CIRCUITS UP TO A STARTING EDGE, AND NEEDS BALANCE FIRST
// #
// #  ec(G) = arborescences rooted at ANY fixed vertex * prod over v of
// #          (outdeg(v) - 1)!
// #
// #  Preconditions: every vertex has indeg == outdeg, and the graph is
// #  connected on its edge-bearing vertices. The count is of Eulerian
// #  CIRCUITS as edge sequences up to rotation; multiply by outdeg(root) if
// #  the problem distinguishes the starting edge. The arborescence count is
// #  Matrix-Tree on the DIRECTED Laplacian -- graph/kirchhoff.cpp.
// ############################################################################
//
// PITFALLS:
//   Ryser is O(2^n * n) and exact; the permanent has no polynomial algorithm.
//     n <= 20 comfortably, n = 22 pushing it.
//   Ryser's signs alternate with the parity of the EXCLUDED set size. Under a
//     modulus, add mod before storing.
//   Hook length: the hook of cell (i,j) is (arm to the right) + (leg below)
//     + 1 for the cell itself. Forgetting the +1 is the standard error.
//   Fuss-Catalan with k = 1 must reduce to the ordinary Catalan numbers --
//     check that before trusting it.
//   LGV needs the path counts themselves, usually binomials for grid paths.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   count non-crossing tuples of lattice paths      LGV determinant
//   k non-intersecting monotone paths in a grid      LGV with binomial entries
//   plane partitions / rhombus tilings               LGV or the hook formula
//   count Eulerian circuits of a digraph             BEST + Matrix-Tree
//   number of de Bruijn sequences of order k          BEST on the de Bruijn
//                                                    graph
//   permanent of a 0/1 matrix                        Ryser
//   "assign one per row and column", weighted count   Ryser (counting), or
//                                                    Hungarian (optimising) --
//                                                    graph/matching/hungarian.cpp
//   count perfect matchings in a bipartite graph      = the permanent = Ryser
//   number of standard Young tableaux of a shape      hook_length()
//   ballot / Catalan generalised to k candidates      hook length on a
//                                                    rectangular shape
//   k-ary trees with n internal nodes                 Fuss-Catalan
//   paths staying above the line y = x/k              Fuss-Catalan
//   spanning trees of an undirected graph             Kirchhoff --
//                                                    graph/kirchhoff.cpp
//
// NOT THIS:
//   paths that MAY cross     -> just multiply the individual counts.
//   the determinant of a 0/1 matrix is not the permanent -- signs matter, and
//     the two differ for n >= 2.
// ============================================================================
const ll MOD = 1000000007;
const int MX = 2000005;

ll pw(ll b, ll e, ll m = MOD) {
    ll r = 1; b %= m;
    while (e) { if (e & 1) r = r * b % m; b = b * b % m, e >>= 1; }
    return r;
}
ll fact[MX], ifact[MX];
void init_fact(int n) {
    fact[0] = 1;
    for (int i = 1; i <= n; i++) fact[i] = fact[i - 1] * i % MOD;
    ifact[n] = pw(fact[n], MOD - 2);
    for (int i = n; i >= 1; i--) ifact[i - 1] = ifact[i] * i % MOD;
}
ll C(ll n, ll r) {
    if (r < 0 || r > n || n < 0) return 0;
    return fact[n] * ifact[r] % MOD * ifact[n - r] % MOD;
}

// determinant mod a prime, O(n^3). math/gauss.cpp has the same routine.
ll det_mod(vector<vector<ll>> a) {
    int n = a.size();
    ll res = 1;
    for (int i = 0; i < n; i++) {
        int p = -1;
        for (int j = i; j < n; j++) if (a[j][i]) { p = j; break; }
        if (p < 0) return 0;
        if (p != i) swap(a[p], a[i]), res = MOD - res;
        res = res * a[i][i] % MOD;
        ll inv = pw(a[i][i], MOD - 2);
        for (int j = i + 1; j < n; j++) {
            ll f = a[j][i] * inv % MOD;
            if (!f) continue;
            for (int k = i; k < n; k++)
                a[j][k] = (a[j][k] - f * a[i][k] % MOD + MOD) % MOD;
        }
    }
    return res;
}

// ---- LGV: M[i][j] = number of paths from start i to end j -----------------
// The caller supplies M. Read the box before trusting the result as a count.
ll lgv(vector<vector<ll>> M) { return det_mod(M); }

// grid-path helper: monotone paths from (x1,y1) to (x2,y2), right/up only
ll grid_paths(ll x1, ll y1, ll x2, ll y2) {
    if (x2 < x1 || y2 < y1) return 0;
    return C((x2 - x1) + (y2 - y1), x2 - x1);
}

// ---- RYSER: permanent of an n x n matrix, O(2^n * n) ----------------------
//   perm(A) = (-1)^n * sum over subsets S of (-1)^|S| * prod_i sum_{j in S} A[i][j]
ll permanent(vector<vector<ll>> &A) {
    int n = A.size();
    ll total = 0;
    vector<ll> rowsum(n, 0);
    for (int S = 1; S < (1 << n); S++) {
        int b = __builtin_ctz(S);
        // incremental: S differs from S ^ (1<<b) by adding column b
        // (rebuild plainly for clarity; O(2^n * n) either way)
        for (int i = 0; i < n; i++) {
            ll s = 0;
            for (int j = 0; j < n; j++) if (S >> j & 1) s += A[i][j];
            rowsum[i] = s % MOD;
        }
        (void)b;
        ll prod = 1;
        for (int i = 0; i < n; i++) prod = prod * rowsum[i] % MOD;
        if ((n - __builtin_popcount(S)) & 1) total = (total - prod + MOD) % MOD;
        else                                 total = (total + prod) % MOD;
    }
    return total;
}

// ---- HOOK LENGTH: standard Young tableaux of shape lambda -----------------
//   n! / product over cells of hook(cell),   hook = arm + leg + 1
ll hook_length(vector<int> lam) {
    int n = 0;
    for (int x : lam) n += x;
    // conjugate shape: col[j] = how many rows reach column j
    int W = lam.empty() ? 0 : lam[0];
    vector<int> col(W, 0);
    for (int x : lam) for (int j = 0; j < x; j++) col[j]++;
    ll denom = 1;
    for (int i = 0; i < (int)lam.size(); i++)
        for (int j = 0; j < lam[i]; j++) {
            ll arm = lam[i] - j - 1;             // cells to the right
            ll leg = col[j] - i - 1;             // cells below
            denom = denom * ((arm + leg + 1) % MOD) % MOD;   // +1 = the cell
        }
    return fact[n] * pw(denom, MOD - 2) % MOD;
}

// ---- FUSS-CATALAN: A(n, k) = C(kn + n, n) / (kn + 1) ----------------------
// k = 1 gives the ordinary Catalan numbers. Counts k-ary trees with n internal
// nodes, and lattice paths staying weakly above a slope-1/k line.
ll fuss_catalan(ll n, ll k) {
    return C(k * n + n, n) * pw((k * n + 1) % MOD, MOD - 2) % MOD;
}

////////////////////////////////////////////////////////////////////////////////
// BEST THEOREM -- Eulerian circuits of a directed graph
//
//   ec(G) = tw(G, r) * product over v of (outdeg(v) - 1)!
//
// tw(G, r) = number of arborescences ORIENTED TOWARDS r, which is any cofactor
// of the directed Laplacian L = D_out - A. Preconditions: indeg == outdeg at
// every vertex, and connectivity on edge-bearing vertices. The result counts
// circuits up to the choice of starting edge; multiply by outdeg(r) if the
// starting edge is distinguished.
//
// See graph/kirchhoff.cpp for the undirected Matrix-Tree version.
////////////////////////////////////////////////////////////////////////////////
ll best_theorem(int n, vector<vector<int>> &cntEdge, int root) {
    vector<int> outd(n, 0), ind(n, 0);
    for (int u = 0; u < n; u++)
        for (int v = 0; v < n; v++) outd[u] += cntEdge[u][v], ind[v] += cntEdge[u][v];
    for (int v = 0; v < n; v++) if (outd[v] != ind[v]) return 0;   // not Eulerian

    // ISOLATED VERTICES MUST BE DROPPED. A vertex with outdeg 0 contributes an
    // all-zero Laplacian row and forces the determinant to 0, reporting "no
    // Eulerian circuit" for a graph that has one.
    vector<int> keep;
    for (int v = 0; v < n; v++) if (outd[v] > 0 && v != root) keep.push_back(v);

    // directed Laplacian over `keep` (root already deleted), determinant
    vector<vector<ll>> L;
    for (int i : keep) {
        vector<ll> row;
        for (int j : keep) {
            ll v = (i == j) ? outd[i] : -cntEdge[i][j];
            row.push_back(((v % MOD) + MOD) % MOD);
        }
        L.push_back(row);
    }
    ll tw = L.empty() ? 1 : det_mod(L);
    ll res = tw;
    for (int v = 0; v < n; v++)
        if (outd[v] > 0) res = res * fact[outd[v] - 1] % MOD;
    return res;
}
