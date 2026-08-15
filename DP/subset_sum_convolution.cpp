// ============================================================================
// SUBSET-SUM CONVOLUTION -- sum over DISJOINT pairs, O(2^n * n^2)
// ----------------------------------------------------------------------------
//     h[S] = sum over A + B = S with A and B DISJOINT of f[A] * g[B]
//
// The plain OR-convolution (zeta, multiply, Mobius) computes
//     sum over A | B = S of f[A]*g[B]
// which OVER-COUNTS: it accepts pairs that overlap. The fix is a rank
// dimension.
//
//     fhat[k][S] = f[S] if popcount(S) == k else 0,   zeta-transformed
//
// Then convolve the ranks like polynomials: hhat[k] = sum_{i+j=k} fhat[i] *
// ghat[j] pointwise. Finally Mobius each rank back and read
//     h[S] = hhat[popcount(S)][S]
//
// Why it works: |A| + |B| == |A | B| holds exactly when A and B are disjoint.
// The rank index enforces disjointness arithmetically, and everything else is
// the OR-convolution you already have.
//
// ############################################################################
// #  READ hhat AT rank == popcount(S), NOT AT THE TOP RANK
// #
// #  hhat[k][S] for k > popcount(S) contains the overlapping garbage that the
// #  rank trick is there to discard. Reading the last row, or summing over
// #  all k, reproduces exactly the over-counted OR-convolution you were
// #  trying to avoid.
// ############################################################################
//
// ############################################################################
// #  ZETA AND MOBIUS ARE PER RANK, NOT ACROSS RANKS
// #
// #  There are n+1 independent transforms, one per rank layer. Applying zeta
// #  to the whole (n+1) x 2^n array as if it were flat mixes ranks and the
// #  result is meaningless. The pointwise multiply is the ONLY step that
// #  couples ranks, and it couples them as a polynomial product in k.
// ############################################################################
//
// PITFALLS:
//   Memory is (n+1) * 2^n values. n = 20 means 21 * 1e6 * 8 bytes = 176 MB --
//     that, not time, is what stops you. n <= 18 is the practical ceiling.
//   Time is 2^n * n^2. n = 20 is 4e8; n = 18 is 8.5e7 and comfortable.
//   Under a modulus, reduce after the inner accumulation, not every term.
//   Without a modulus, the values are sums of 2^n products -- overflow is real.
//   f[0] and g[0] participate: the empty set is a legal operand.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   h[S] = sum over disjoint A+B=S of f[A]g[B]      subset_sum_convolution()
//   partition a set into groups with arbitrary       repeated convolution, or
//     per-group cost, n up to ~20                    exp of a set power series;
//                                                    beats 3^n around n = 20 --
//                                                    below n = 18 prefer
//                                                    DP/submask_partition.cpp
//   "cover S exactly once by k pieces"               convolve f with itself k
//                                                    times
//   count ways to tile / cover with disjoint parts   this
//   OR-convolution (overlaps allowed)                zeta, multiply, Mobius --
//                                                    no rank dimension needed
//   AND-convolution                                  superset zeta, same shape
//   XOR-convolution                                  FWHT -- math/fwht.cpp
//   "for each mask, aggregate over submasks"         plain SOS -- DP/sos.cpp
//   chromatic polynomial / graph colouring           inclusion-exclusion over
//                                                    independent sets --
//                                                    graph/graph_coloring.cpp
//
// NOT THIS:
//   overlapping pairs are fine       -> OR-convolution, drop the rank, O(2^n n).
//   n <= 18 and you only need ONE partition DP -> the 3^n submask loop is
//     simpler and often faster in practice. DP/submask_partition.cpp.
//   the index is a SUM of integers, not a set union -> FFT, math/fft_ntt.cpp.
// ============================================================================
const ll MOD = 1000000007;

// zeta over subsets, in place, one layer
void zeta(vector<ll> &a, int n) {
    for (int i = 0; i < n; i++)
        for (int m = 0; m < (1 << n); m++)
            if (m >> i & 1) a[m] = (a[m] + a[m ^ (1 << i)]) % MOD;
}
// Mobius = the same loop with a minus
void mobius(vector<ll> &a, int n) {
    for (int i = 0; i < n; i++)
        for (int m = 0; m < (1 << n); m++)
            if (m >> i & 1) a[m] = (a[m] - a[m ^ (1 << i)] + MOD) % MOD;
}

vector<ll> subset_sum_convolution(vector<ll> &f, vector<ll> &g, int n) {
    int N = 1 << n;
    // split by popcount, then zeta each layer independently
    vector<vector<ll>> F(n + 1, vector<ll>(N, 0)), G(n + 1, vector<ll>(N, 0));
    for (int m = 0; m < N; m++) {
        F[__builtin_popcount(m)][m] = f[m] % MOD;
        G[__builtin_popcount(m)][m] = g[m] % MOD;
    }
    for (int k = 0; k <= n; k++) zeta(F[k], n), zeta(G[k], n);

    // polynomial multiply in the rank index, pointwise in the mask index
    vector<vector<ll>> H(n + 1, vector<ll>(N, 0));
    for (int k = 0; k <= n; k++)
        for (int i = 0; i <= k; i++)
            for (int m = 0; m < N; m++)
                H[k][m] = (H[k][m] + F[i][m] * G[k - i][m]) % MOD;

    for (int k = 0; k <= n; k++) mobius(H[k], n);

    vector<ll> h(N);
    for (int m = 0; m < N; m++)
        h[m] = H[__builtin_popcount(m)][m];        // the rank that matters
    return h;
}
