// ============================================================================
// BERLEKAMP-MASSEY + KITAMASA -- find the recurrence, then jump to term 1e18
// ----------------------------------------------------------------------------
// Two halves that are almost always used together:
//
//   BERLEKAMP-MASSEY   given the first terms of a sequence, recover the
//                      shortest linear recurrence
//                          a[n] = c[0]a[n-1] + c[1]a[n-2] + ... + c[d-1]a[n-d]
//                      O(n^2). You never have to derive the recurrence.
//
//   KITAMASA           given c[] and the first d terms, compute a[k] for
//                      k up to 1e18 in O(d^2 log k) -- polynomial powering
//                      modulo the characteristic polynomial.
//
// Together: brute-force ~2d terms of ANY DP with a fixed finite state and
// linear transitions, and you get term 1e18 without ever writing a matrix.
// Compare O(d^3 log k) for matrix power -- Kitamasa wins from d ~ 30 up.
//
// ############################################################################
// #  FEED IT AT LEAST 2d TERMS, AND PREFERABLY MORE
// #
// #  BM returns the shortest recurrence CONSISTENT WITH THE INPUT. With only
// #  2d-1 terms it can return a shorter, wrong one that reproduces every
// #  value you gave it and diverges immediately after. There is no error
// #  signal.
// #
// #  Rule: generate 2d + 10 terms if you can, then CHECK -- run the returned
// #  recurrence forward over your own samples and confirm it reproduces the
// #  tail you held back. verify() below does exactly that. Do not skip it.
// ############################################################################
//
// ############################################################################
// #  THE MODULUS MUST BE PRIME
// #
// #  BM inverts the discrepancy. Composite modulus means the inverse may not
// #  exist and the algorithm produces silent garbage. For a composite
// #  modulus, factor it and run per prime power, or work over the rationals.
// #  Zero-only prefixes are fine -- BM handles a leading run of zeros.
// ############################################################################
//
// PITFALLS:
//   Index convention: c[i] multiplies a[n-1-i]. Getting this backwards
//     reverses the recurrence and still runs.
//   a[] must be reduced mod p before BM, and non-negative.
//   The returned degree d can be 0 (all-zero sequence) -- guard k < d by
//     returning a[k] directly.
//   Kitamasa needs at least d initial terms; feed it the same array you gave BM.
//   If your sequence is POLYNOMIAL in n, this works too (a polynomial of
//     degree m satisfies a recurrence of order m+1) -- but Lagrange
//     interpolation is O(d) instead of O(d^2 log k). math/interpolation.cpp.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   "term k of this sequence", k up to 1e18            BM then Kitamasa
//   a DP with a fixed small state and huge n            brute 2d terms, BM,
//                                                       Kitamasa. Works even
//                                                       when you cannot see
//                                                       the recurrence
//   count walks of length k in a small graph            same; d <= number of
//                                                       states
//   count strings of length n avoiding a pattern        KMP/Aho automaton is
//                                                       linear -> BM works
//   nth Fibonacci / Tribonacci / any fixed recurrence   skip BM, Kitamasa
//                                                       directly
//   the sequence is a POLYNOMIAL in n                   Lagrange is cheaper --
//                                                       math/interpolation.cpp
//   you have the recurrence but want a matrix anyway    DP/matrix/matrix expo.cpp
//   determinant of a huge structured matrix             BM on the sequence of
//                                                       determinants by size
//
// NOT THIS:
//   the transition coefficients DEPEND ON n  -> not linear-recurrent; neither
//     BM nor matrix power applies.
//   the state count is not bounded            -> there is no finite recurrence.
//   you only need small n                     -> just run the DP.
// ============================================================================
const ll MOD = 1000000007;

ll pw(ll b, ll e, ll m = MOD) {
    ll r = 1; b %= m;
    while (e) { if (e & 1) r = r * b % m; b = b * b % m, e >>= 1; }
    return r;
}
ll inv(ll a) { return pw(a, MOD - 2); }

// ---- Berlekamp-Massey: shortest c[] with a[n] = sum c[i]*a[n-1-i] --------
vector<ll> berlekamp_massey(const vector<ll> &a) {
    vector<ll> ls, cur;                 // last failing candidate, current
    int lf = 0;                         // index where ls last failed
    ll ld = 0;                          // the discrepancy at that failure
    for (int i = 0; i < (int)a.size(); i++) {
        ll t = 0;
        for (int j = 0; j < (int)cur.size(); j++)
            t = (t + cur[j] * a[i - 1 - j]) % MOD;
        if ((a[i] - t) % MOD == 0) continue;             // prediction was right
        if (cur.empty()) {                               // first failure
            cur.assign(i + 1, 0);
            lf = i, ld = (a[i] - t) % MOD;
            continue;
        }
        ll k = (a[i] - t) % MOD * inv(ld) % MOD;
        vector<ll> c(i - lf - 1);                        // shift by the gap
        c.push_back(k);
        for (int j = 0; j < (int)ls.size(); j++)
            c.push_back(-ls[j] * k % MOD);
        if (c.size() < cur.size()) c.resize(cur.size());
        for (int j = 0; j < (int)cur.size(); j++)
            c[j] = (c[j] + cur[j]) % MOD;
        if (i - (int)cur.size() >= lf - (int)ls.size())  // keep the better one
            ls = cur, lf = i, ld = (a[i] - t) % MOD;
        cur = c;
    }
    for (ll &x : cur) x = (x % MOD + MOD) % MOD;
    return cur;
}

// ---- run the recurrence forward and check it reproduces `a` --------------
// ALWAYS call this. It is the only defence against a too-short recurrence.
bool verify(const vector<ll> &a, const vector<ll> &c) {
    int d = c.size();
    if (d == 0) {
        for (ll x : a) if (x % MOD) return false;
        return true;
    }
    for (int i = d; i < (int)a.size(); i++) {
        ll t = 0;
        for (int j = 0; j < d; j++) t = (t + c[j] * a[i - 1 - j]) % MOD;
        if ((t - a[i]) % MOD) return false;
    }
    return true;
}

// ---- polynomial multiply modulo the characteristic polynomial ------------
// char poly: x^d - c[0]x^(d-1) - ... - c[d-1]
vector<ll> poly_mul_mod(const vector<ll> &x, const vector<ll> &y,
                        const vector<ll> &c) {
    int d = c.size();
    vector<ll> r(x.size() + y.size() - 1, 0);
    for (size_t i = 0; i < x.size(); i++)
        for (size_t j = 0; j < y.size(); j++)
            r[i + j] = (r[i + j] + x[i] * y[j]) % MOD;
    // reduce from the top: x^n = sum c[j] * x^(n-1-j)
    for (int i = (int)r.size() - 1; i >= d; i--) {
        if (!r[i]) continue;
        for (int j = 0; j < d; j++)
            r[i - 1 - j] = (r[i - 1 - j] + r[i] * c[j]) % MOD;
        r[i] = 0;
    }
    r.resize(d);
    return r;
}

// ---- Kitamasa: a[k] from the recurrence c[] and the first d terms --------
ll kitamasa(const vector<ll> &a, const vector<ll> &c, ll k) {
    int d = c.size();
    if (d == 0) return 0;
    if (k < d) return a[k] % MOD;
    vector<ll> res{1}, base{0, 1};                       // res = 1, base = x
    if (d == 1) base = {c[0]};                           // x reduces immediately
    while (k) {
        if (k & 1) res = poly_mul_mod(res, base, c);
        base = poly_mul_mod(base, base, c);
        k >>= 1;
    }
    ll r = 0;
    for (int i = 0; i < d && i < (int)res.size(); i++)
        r = (r + res[i] * (a[i] % MOD)) % MOD;
    return r;
}

// ---- the whole pipeline: samples in, a[k] out. Returns -1 if unverified. --
ll nth_term(const vector<ll> &a, ll k) {
    vector<ll> c = berlekamp_massey(a);
    if (!verify(a, c)) return -1;                        // needs more samples
    if ((int)a.size() < 2 * (int)c.size()) return -1;    // too few to trust
    return kitamasa(a, c, k);
}
