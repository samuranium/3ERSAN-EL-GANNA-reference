// ============================================================================
// LAGRANGE INTERPOLATION -- a degree-d polynomial from d+1 points
// ----------------------------------------------------------------------------
// If f is a polynomial of degree <= d and you can compute f(0..d), you can
// evaluate f ANYWHERE. Two versions:
//
//   general points  O(d^2) per query
//   CONSECUTIVE points x = 0,1,...,d  O(d) per query, using prefix/suffix
//                     products of (x - i) and the fact that the denominators
//                     are just factorials with alternating signs
//
// Use it whenever a quantity "looks polynomial in n" and n is astronomically
// large: brute-force the first d+1 values, then interpolate. The commonest
// case by far is `sum of i^k for i = 1..n`, which is a polynomial in n of
// degree k+1.
//
// ############################################################################
// #  GET THE DEGREE RIGHT, AND IT IS ALMOST ALWAYS ONE MORE THAN YOU THINK
// #
// #      sum_{i=1..n} i^k   is degree k+1, so it needs k+2 points.
// #
// #  One point short does not error -- it silently fits a lower-degree
// #  polynomial through your data and returns a plausible wrong number.
// #  VERIFY by finite differences: take the (d+1)-th difference of your
// #  sample values; it must be identically 0. If it is not, either the degree
// #  is higher or the quantity is not polynomial.
// ############################################################################
//
// ############################################################################
// #  THIS NEEDS p > d
// #
// #  The denominators are products of (i - j) for i, j <= d. If p <= d one of
// #  those is 0 mod p and the inverse does not exist. Fine for p = 1e9+7 and
// #  any sane d; fatal for a small modulus.
// #
// #  Also: if the query x happens to be one of the sample points, return the
// #  sample directly -- the general formula divides by zero there.
// ############################################################################
//
// PITFALLS:
//   Precompute factorials and inverse factorials once, not per query.
//   The sign is (-1)^(d-i); with a modulus, add mod before storing.
//   Interpolating at x < d (inside the sample range) works but is pointless --
//     just index the array.
//   For "sum of i^k", the sample values y[j] = sum_{i=1..j} i^k must be
//     PREFIX SUMS, not i^k itself.
//   The x values must be distinct. Duplicated sample points give a 0
//     denominator.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   f is polynomial of degree d, need f(n), n huge     lagrange_consecutive()
//   sum of i^k for i = 1..n, n up to 1e18              sum_powers() -- degree
//                                                      k+1
//   "the answer looks smooth in n"                     sample, verify by finite
//                                                      differences, interpolate
//   a counting DP whose answer is polynomial in the    same recipe; this is how
//   grid size                                          most "huge n" counting
//                                                      problems fall
//   fit through ARBITRARY points                       lagrange_general(),
//                                                      O(d^2)
//   the answer is a LINEAR RECURRENCE, not polynomial  different tool --
//                                                      math/linear_recurrence.cpp
//   sum of i^k * r^i (geometric weight)                NOT polynomial. It is
//                                                      polynomial * r^n; use
//                                                      the recurrence, or
//                                                      differentiate the
//                                                      geometric series
//
// NOT THIS:
//   the sequence is exponential or has a recurrence -> Berlekamp-Massey,
//     math/linear_recurrence.cpp.
//   you need the COEFFICIENTS, not evaluations      -> build them by Newton's
//     divided differences, O(d^2). Interpolation here only evaluates.
// ============================================================================
const ll MOD = 1000000007;

ll pw(ll b, ll e, ll m = MOD) {
    ll r = 1; b %= m;
    while (e) { if (e & 1) r = r * b % m; b = b * b % m, e >>= 1; }
    return r;
}
ll inv(ll a) { return pw(a, MOD - 2); }

// ---- CONSECUTIVE sample points: y[i] = f(i) for i = 0..d, evaluate f(x) ---
// O(d) per query after an O(d) factorial precompute.
ll lagrange_consecutive(vector<ll> &y, ll x) {
    int d = (int)y.size() - 1;
    if (x <= d) return y[x];                       // inside the sample
    vector<ll> fact(d + 2), ifact(d + 2);
    fact[0] = 1;
    for (int i = 1; i <= d + 1; i++) fact[i] = fact[i - 1] * i % MOD;
    ifact[d + 1] = inv(fact[d + 1]);
    for (int i = d + 1; i >= 1; i--) ifact[i - 1] = ifact[i] * i % MOD;

    // pre[i] = prod_{j<i} (x - j),  suf[i] = prod_{j>i} (x - j)
    vector<ll> pre(d + 2, 1), suf(d + 2, 1);
    for (int i = 0; i <= d; i++) pre[i + 1] = pre[i] * ((x - i) % MOD) % MOD;
    for (int i = d; i >= 0; i--) suf[i] = suf[i + 1] * ((x - i) % MOD) % MOD;

    ll res = 0;
    for (int i = 0; i <= d; i++) {
        ll num = pre[i] * suf[i + 1] % MOD;
        ll den = ifact[i] * ifact[d - i] % MOD;
        ll term = y[i] % MOD * num % MOD * den % MOD;
        if ((d - i) & 1) res = (res - term + MOD) % MOD;   // sign (-1)^(d-i)
        else             res = (res + term) % MOD;
    }
    return res;
}

// ---- ARBITRARY sample points, O(d^2) -------------------------------------
ll lagrange_general(vector<ll> &xs, vector<ll> &ys, ll x) {
    int n = xs.size();
    ll res = 0;
    for (int i = 0; i < n; i++) {
        if (x % MOD == xs[i] % MOD) return ys[i] % MOD;
        ll num = 1, den = 1;
        for (int j = 0; j < n; j++) {
            if (i == j) continue;
            num = num * ((x - xs[j]) % MOD + MOD) % MOD;
            den = den * ((xs[i] - xs[j]) % MOD + MOD) % MOD;
        }
        res = (res + ys[i] % MOD * num % MOD * inv(den)) % MOD;
    }
    return res;
}

// ---- sum_{i=1..n} i^k mod p, n up to 1e18. Degree is k+1, so k+2 points. --
ll sum_powers(ll n, int k) {
    vector<ll> y(k + 2);
    y[0] = 0;
    for (int i = 1; i <= k + 1; i++) y[i] = (y[i - 1] + pw(i, k)) % MOD;
    return lagrange_consecutive(y, n);          // n is used raw, not reduced:
                                                // the (x - i) terms reduce it
}

// ---- the degree check: the (d+1)-th finite difference must vanish ---------
// Run this on your sample before trusting an interpolation.
bool degree_at_most(vector<ll> y, int d) {
    for (int r = 0; r <= d; r++) {
        if ((int)y.size() < 2) return true;
        vector<ll> nx(y.size() - 1);
        for (size_t i = 0; i + 1 < y.size(); i++)
            nx[i] = ((y[i + 1] - y[i]) % MOD + MOD) % MOD;
        y = nx;
    }
    for (ll v : y) if (v % MOD) return false;
    return true;
}
