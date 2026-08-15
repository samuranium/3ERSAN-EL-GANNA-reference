// ============================================================================
// NUMBER-THEORY ODDS AND ENDS -- Tonelli-Shanks, Josephus, Zeckendorf,
// Frobenius, Stern-Brocot, Pell
// ----------------------------------------------------------------------------
// Six small results that are each one function, each unrecognisable from the
// statement, and none of which are in the printed Number Theory PDF.
// The sieve / Lucas / Miller-Rabin / CRT family lives there; this file is the
// remainder.
//
// ############################################################################
// #  a^((p+1)/4) IS NOT A SQUARE ROOT TEST
// #
// #  When p = 3 (mod 4) that shortcut RETURNS A VALUE for every a, including
// #  the non-residues -- it just is not a root of a. Always test the Legendre
// #  symbol first:  a^((p-1)/2) == 1  means a is a residue, p-1 means it is
// #  not. Skipping the test is the classic silent wrong answer here.
// ############################################################################
//
// ############################################################################
// #  FROBENIUS ab-a-b IS FOR EXACTLY TWO COPRIME COINS
// #
// #  Three or more coins has NO closed form. Model it as a shortest path over
// #  residues mod the smallest coin (the "coin graph" / Round-Robin
// #  algorithm) -- frobenius_multi() below. Applying the two-coin formula to
// #  three coins produces a plausible number that is simply wrong.
// #  Also: gcd of all coins must be 1, or infinitely many amounts are unpayable.
// ############################################################################
//
// PITFALLS:
//   Josephus: the recurrence J(n,k) = (J(n-1,k) + k) mod n is 0-INDEXED. Add 1
//     at the end for 1-indexed people. It is O(n); the O(k log n) version is
//     for n up to 1e18 with small k.
//   Zeckendorf: greedy from the largest Fibonacci down. The representation
//     uses no two CONSECUTIVE Fibonacci numbers, and that is what makes it
//     unique.
//   Stern-Brocot: single steps are O(p+q) and will TLE. Jump whole runs with
//     a division -- that is the difference between O(log) and O(n).
//   Pell: the fundamental solution can be astronomically large (D = 61 gives
//     x = 1766319049). Use __int128 or big integers if D can be adversarial.
//   Tonelli-Shanks is O(log^2 p) worst case; for p = 3 mod 4 the shortcut is
//     O(log p) and worth keeping.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   x^2 = a (mod p), p prime                        sqrt_mod()
//   is a a quadratic residue mod p                   legendre()
//   x^2 = a (mod p^k) or composite                   lift with Hensel, or CRT
//                                                    over the factorisation
//   people in a circle, every k-th eliminated         josephus()
//   ... n up to 1e18, k small                         josephus_fast()
//   "the last survivor"                               same
//   decompose n into non-consecutive Fibonaccis       zeckendorf()
//   Fibonacci Nim losing condition                    the Zeckendorf smallest
//                                                    term
//   largest unpayable amount, 2 coprime coins         a*b - a - b
//   ... 3 or more coins                               frobenius_multi()
//   how many amounts are unpayable, 2 coins           (a-1)(b-1)/2
//   best rational approximation with q <= Q           stern_brocot() /
//                                                    continued fractions
//   the fraction in (lo, hi) with the smallest        stern_brocot_between()
//     denominator
//   x^2 - D y^2 = 1                                   pell()
//   "the smallest integer solution to this quadratic" often Pell in disguise
//
// NOT THIS:
//   primality / factorisation      -> Miller-Rabin, Pollard rho: their
//                                     Number Theory PDF.
//   nCr mod p, CRT, phi, Mobius     -> also the PDF, and math/ here.
//   linear Diophantine ax + by = c  -> math/extgcd.cpp.
// ============================================================================
ll pw(ll b, ll e, ll m) {
    ll r = 1; b %= m; if (b < 0) b += m;
    while (e) { if (e & 1) r = (__int128)r * b % m; b = (__int128)b * b % m, e >>= 1; }
    return r;
}

// ---- Legendre symbol: 1 residue, -1 non-residue, 0 if p | a ---------------
int legendre(ll a, ll p) {
    a %= p; if (a < 0) a += p;
    if (!a) return 0;
    return pw(a, (p - 1) / 2, p) == 1 ? 1 : -1;
}

// ---- Tonelli-Shanks: one square root of a mod an odd prime p, or -1 -------
ll sqrt_mod(ll a, ll p) {
    a %= p; if (a < 0) a += p;
    if (!a) return 0;
    if (p == 2) return a;
    if (legendre(a, p) != 1) return -1;              // TEST FIRST. See the box.
    if (p % 4 == 3) return pw(a, (p + 1) / 4, p);    // the shortcut, now safe

    ll q = p - 1, s = 0;
    while (q % 2 == 0) q /= 2, s++;
    ll z = 2;
    while (legendre(z, p) != -1) z++;                // any non-residue
    ll m = s, c = pw(z, q, p), t = pw(a, q, p), r = pw(a, (q + 1) / 2, p);
    while (t != 1) {
        ll i = 0, tt = t;
        while (tt != 1) tt = (__int128)tt * tt % p, i++;
        ll b = c;
        for (ll j = 0; j < m - i - 1; j++) b = (__int128)b * b % p;
        m = i;
        c = (__int128)b * b % p;
        t = (__int128)t * c % p;
        r = (__int128)r * b % p;
    }
    return r;                                        // p - r is the other root
}

// ---- Josephus: 0-indexed survivor among n people, every k-th removed ------
int josephus(int n, int k) {
    int r = 0;
    for (int i = 2; i <= n; i++) r = (r + k) % i;
    return r;                                        // add 1 for 1-indexed
}
// n up to 1e18 with small k: skip whole blocks of eliminations at once
ll josephus_fast(ll n, ll k) {
    if (k == 1) return n - 1;
    ll r = 0;
    for (ll i = 2; i <= n; ) {
        if (r + k >= i) { r = (r + k) % i; i++; continue; }
        ll blocks = (i - r - 1) / (k - 1);           // how many steps fit
        if (i + blocks > n) blocks = n - i + 1;
        r += blocks * k;
        i += blocks;
        r %= i - 1;
    }
    return r;
}

// ---- Zeckendorf: n as a sum of non-consecutive Fibonacci numbers ----------
vector<ll> zeckendorf(ll n) {
    vector<ll> f{1, 2};
    while (f.back() <= n) f.push_back(f[f.size() - 1] + f[f.size() - 2]);
    vector<ll> res;
    for (int i = (int)f.size() - 1; i >= 0 && n > 0; i--)
        if (f[i] <= n) res.push_back(f[i]), n -= f[i];   // greedy from the top
    return res;                                          // descending
}

// ---- Frobenius: largest amount NOT payable -------------------------------
ll frobenius2(ll a, ll b) { return a * b - a - b; }      // gcd(a,b) == 1 only
ll unpayable_count2(ll a, ll b) { return (a - 1) * (b - 1) / 2; }

// 3+ coins: shortest path over residues mod the smallest coin.
// d[r] = smallest payable amount that is r (mod c0). Answer = max d[r] - c0.
ll frobenius_multi(vector<ll> coins) {
    sort(coins.begin(), coins.end());
    ll g = 0;
    for (ll c : coins) g = __gcd(g, c);
    if (g != 1) return -1;                           // infinitely many unpayable
    ll c0 = coins[0];
    if (c0 == 1) return -1;                          // everything is payable
    vector<ll> d(c0, LLONG_MAX);
    d[0] = 0;
    priority_queue<pair<ll, ll>, vector<pair<ll, ll>>, greater<>> pq;
    pq.push({0, 0});
    while (!pq.empty()) {
        auto [cur, r] = pq.top(); pq.pop();
        if (cur > d[r]) continue;
        for (size_t i = 1; i < coins.size(); i++) {
            ll nr = (r + coins[i]) % c0, nd = cur + coins[i];
            if (nd < d[nr]) d[nr] = nd, pq.push({nd, nr});
        }
    }
    ll best = 0;
    for (ll x : d) if (x != LLONG_MAX) best = max(best, x);
    return best - c0;
}

// ---- Stern-Brocot: the fraction in (lo, hi) with the smallest denominator -
// Strictly between ln/ld and hn/hd. Non-negative, ln/ld < hn/hd. O(log) --
// each level strips the integer part and flips to the reciprocal interval,
// which is exactly the continued-fraction descent. Single Stern-Brocot steps
// would be O(p+q); this jumps whole runs with one division.
pair<ll, ll> stern_brocot_between(ll ln, ll ld, ll hn, ll hd) {
    ll k = ln / ld;                                  // strip the integer part
    ln -= k * ld, hn -= k * hd;                      // now 0 <= ln/ld < 1
    if (hn > hd)                                     // an integer fits inside
        return {k + 1, 1};
    if (ln == 0) {                                   // interval is (0, hn/hd)
        ll q = hd / hn + 1;                          // smallest q with 1/q < hn/hd
        return {k * q + 1, q};
    }
    // reciprocal: 1/x lies in (hd/hn, ld/ln)
    auto [p, q] = stern_brocot_between(hd, hn, ld, ln);
    return {k * p + q, p};                           // x = q/p, plus k
}

// ---- Pell: fundamental solution of x^2 - D y^2 = 1, D not a perfect square
// From the continued-fraction expansion of sqrt(D). Values grow FAST.
pair<__int128, __int128> pell(ll D) {
    ll a0 = (ll)sqrtl((long double)D);
    while (a0 * a0 > D) a0--;
    while ((a0 + 1) * (a0 + 1) <= D) a0++;
    if (a0 * a0 == D) return {0, 0};                 // perfect square: no solution
    ll m = 0, d = 1, a = a0;
    __int128 num1 = 1, num = a0, den1 = 0, den = 1;
    while ((__int128)num * num - (__int128)D * den * den != 1) {
        m = d * a - m;
        d = (D - m * m) / d;
        a = (a0 + m) / d;
        __int128 nn = a * num + num1, dn = a * den + den1;
        num1 = num, num = nn;
        den1 = den, den = dn;
    }
    return {num, den};
}
