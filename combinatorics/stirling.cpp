// ============================================================================
// STIRLING / BELL / DERANGEMENTS / TWELVEFOLD WAY -- the "balls in boxes" file
// ----------------------------------------------------------------------------
// Almost every counting statement is one of twelve problems. Fix these four
// labels FIRST and the formula follows mechanically:
//
//     are the BALLS distinguishable?   are the BOXES distinguishable?
//     any / injective / surjective?
//
// ############################################################################
// #  THE TWELVEFOLD WAY -- n balls into k boxes
// #
// #   balls   boxes   any                     injective    surjective
// #   -----   -----   ---------------------   ----------   -------------------
// #   dist    dist    k^n                     k!/(k-n)!    k! * S2(n,k)
// #   same    dist    C(n+k-1, n)             C(k, n)      C(n-1, n-k)
// #   dist    same    sum_{j=1..k} S2(n,j)    [n <= k]     S2(n,k)
// #   same    same    p(n, k) partitions      [n <= k]     p(n-k) partitions
// #
// #  "distinct balls, identical boxes, any" is the BELL number when k >= n.
// #  Getting the labels backwards is the single most common counting error,
// #  and every one of these formulas is a plausible-looking wrong answer for
// #  the other cells.
// ############################################################################
//
// ############################################################################
// #  S2(n,k) BY THE EXPLICIT SUM NEEDS THE SIGN, AND IT ALTERNATES
// #
// #      S2(n,k) = (1/k!) * sum_{j=0}^{k} (-1)^j C(k,j) (k-j)^n
// #
// #  Under a modulus every negative term must have `mod` added before it is
// #  stored. Dropping the sign gives a number that is far too large but still
// #  "looks like a count". The O(n*k) recurrence below has no signs at all
// #  and is the safer default when n*k fits.
// ############################################################################
//
// PITFALLS:
//   S2(0,0) = 1, S2(n,0) = 0 for n > 0, S2(0,k) = 0 for k > 0. The empty
//     partition of the empty set is one partition, not zero.
//   Bell(n) = sum_{k=0..n} S2(n,k). Bell(0) = 1.
//   Derangements: D(0) = 1, D(1) = 0. D(1) = 0 is the case that breaks a
//     naive `n! / e` rounding.
//   The unsigned Stirling FIRST kind counts permutations by CYCLE count -- a
//     completely different sequence from the second kind. Do not mix them.
//   Surjections onto k LABELLED boxes is `k! * S2(n,k)`, not S2(n,k).
//   Modulus must be prime for the inverse factorials.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   partition n labelled items into exactly k         S2(n,k)
//     non-empty unlabelled groups
//   ... into ANY number of groups                     Bell(n)
//   surjections from n items onto k labels            k! * S2(n,k)
//   "every box non-empty", boxes labelled             same
//   permutations with no fixed point                  derangements()
//   permutations with exactly k fixed points          C(n,k) * D(n-k)
//   permutations with exactly k CYCLES                Stirling first kind
//   "n people, nobody gets their own hat"             derangements()
//   n identical balls into k labelled boxes           C(n+k-1, k-1), stars and
//                                                     bars
//   ... each box non-empty                            C(n-1, k-1)
//   ... box i holds at most u_i                       inclusion-exclusion over
//                                                     the violated bounds --
//                                                     DP/incexc.cpp
//   partitions of the INTEGER n                       partitions() -- the
//                                                     pentagonal recurrence
//   partitions of n into exactly k parts              p(n,k) table below
//   count set partitions of a SUBSET-cost kind        DP/submask_partition.cpp
//
// NOT THIS:
//   the groups are ordered / the boxes are labelled -> multiply by k!, or use
//     the "dist boxes" row. Half of all errors here are this.
//   n <= 20 and each group has an arbitrary cost    -> DP/submask_partition.cpp.
//   counting up to SYMMETRY (rotations)             -> combinatorics/burnside.cpp.
// ============================================================================
const ll MOD = 1000000007;
const int MX = 2005;

ll pw(ll b, ll e, ll m = MOD) {
    ll r = 1; b %= m;
    while (e) { if (e & 1) r = r * b % m; b = b * b % m, e >>= 1; }
    return r;
}

ll fact[MX], ifact[MX];
void init_fact() {
    fact[0] = 1;
    for (int i = 1; i < MX; i++) fact[i] = fact[i - 1] * i % MOD;
    ifact[MX - 1] = pw(fact[MX - 1], MOD - 2);
    for (int i = MX - 1; i >= 1; i--) ifact[i - 1] = ifact[i] * i % MOD;
}
ll C(int n, int r) {
    if (r < 0 || r > n || n < 0) return 0;
    return fact[n] * ifact[r] % MOD * ifact[n - r] % MOD;
}

// ---- Stirling SECOND kind: partitions of n labelled items into k groups ---
//      S2(n,k) = k*S2(n-1,k) + S2(n-1,k-1)      (put item n in an existing
//                                                group, or start a new one)
ll S2[MX][MX];
void build_stirling2(int n) {
    S2[0][0] = 1;
    for (int i = 1; i <= n; i++)
        for (int j = 1; j <= i; j++)
            S2[i][j] = (S2[i - 1][j] * j + S2[i - 1][j - 1]) % MOD;
}
// one value in O(k log n), no table. Signs alternate -- see the box.
ll stirling2(int n, int k) {
    if (k < 0 || k > n) return (n == 0 && k == 0);
    ll r = 0;
    for (int j = 0; j <= k; j++) {
        ll t = C(k, j) * pw(k - j, n) % MOD;
        if (j & 1) r = (r - t + MOD) % MOD;
        else       r = (r + t) % MOD;
    }
    return r * ifact[k] % MOD;
}

// ---- Stirling FIRST kind (unsigned): permutations of n with k cycles ------
//      c(n,k) = (n-1)*c(n-1,k) + c(n-1,k-1)
ll S1[MX][MX];
void build_stirling1(int n) {
    S1[0][0] = 1;
    for (int i = 1; i <= n; i++)
        for (int j = 1; j <= i; j++)
            S1[i][j] = (S1[i - 1][j] * (i - 1) + S1[i - 1][j - 1]) % MOD;
}

// ---- Bell: total set partitions. Bell(n) = sum_k S2(n,k) ------------------
// The triangle is O(n^2) and needs no factorials.
ll bell(int n) {
    vector<ll> row{1}, nxt;
    for (int i = 0; i < n; i++) {
        nxt.assign(row.size() + 1, 0);
        nxt[0] = row.back();
        for (size_t j = 0; j < row.size(); j++)
            nxt[j + 1] = (nxt[j] + row[j]) % MOD;
        row.swap(nxt);
    }
    return row[0];
}

// ---- derangements: D(n) = (n-1)*(D(n-1) + D(n-2)),  D(0)=1, D(1)=0 -------
vector<ll> derangements(int n) {
    vector<ll> d(max(2, n + 1));
    d[0] = 1, d[1] = 0;
    for (int i = 2; i <= n; i++) d[i] = (ll)(i - 1) * ((d[i - 1] + d[i - 2]) % MOD) % MOD;
    d.resize(n + 1);
    return d;
}
// permutations of n with EXACTLY k fixed points
ll exactly_k_fixed(int n, int k, vector<ll> &d) { return C(n, k) * d[n - k] % MOD; }

////////////////////////////////////////////////////////////////////////////////
// INTEGER PARTITIONS
//
// p(n)     = number of ways to write n as a sum of positive integers,
//            order irrelevant. Pentagonal recurrence, O(n sqrt n):
//              p(n) = sum_k (-1)^(k-1) [ p(n - g(k)) + p(n - g(-k)) ]
//            with g(k) = k(3k-1)/2, and BOTH k and -k are needed.
//            The signs cycle + + - -.
// p(n,k)   = partitions of n into exactly k parts,  O(n*k):
//              p(n,k) = p(n-1, k-1) + p(n-k, k)
//            (a part equal to 1, or subtract 1 from every part)
////////////////////////////////////////////////////////////////////////////////
vector<ll> partitions(int n) {
    vector<ll> p(n + 1, 0);
    p[0] = 1;
    for (int i = 1; i <= n; i++) {
        for (int k = 1;; k++) {
            ll g1 = (ll)k * (3 * k - 1) / 2;             // generalised
            ll g2 = (ll)k * (3 * k + 1) / 2;             // pentagonal, +-k
            if (g1 > i && g2 > i) break;
            ll s = 0;
            if (g1 <= i) s += p[i - g1];
            if (g2 <= i) s += p[i - g2];
            if (k & 1) p[i] = (p[i] + s) % MOD;          // signs: + + - -
            else       p[i] = (p[i] - s % MOD + MOD) % MOD;
        }
    }
    return p;
}
// partitions of n into exactly k parts
vector<vector<ll>> partitions_k(int n, int k) {
    vector<vector<ll>> p(n + 1, vector<ll>(k + 1, 0));
    p[0][0] = 1;
    for (int i = 1; i <= n; i++)
        for (int j = 1; j <= k && j <= i; j++)
            p[i][j] = (p[i - 1][j - 1] + p[i - j][j]) % MOD;
    return p;
}
