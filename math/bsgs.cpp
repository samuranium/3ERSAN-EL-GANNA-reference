// ============================================================================
// BABY-STEP GIANT-STEP -- discrete logarithm, O(sqrt(m) log m)
// ----------------------------------------------------------------------------
// Solve   a^x = b  (mod m)   for the smallest x >= 0.
//
// Write x = p*n - q with n = ceil(sqrt(m)) and 0 <= q < n. Then
//     a^(p*n) = b * a^q  (mod m)
// Store every right-hand side for q in [0, n) in a table (the BABY steps),
// then walk p = 1, 2, ... computing a^(p*n) (the GIANT steps) and look each up.
// Two O(sqrt m) loops instead of one O(m) loop.
//
// ############################################################################
// #  THE COPRIME CASE AND THE GENERAL CASE ARE DIFFERENT FUNCTIONS
// #
// #  gcd(a, m) == 1  ->  bsgs(). Clean, and the only version most write-ups
// #                      give.
// #  gcd(a, m) != 1  ->  bsgs_any(). Repeatedly divide out the common factor,
// #                      tracking a multiplier, until a and the modulus are
// #                      coprime, then call the coprime version.
// #
// #  Using the coprime version on a non-coprime input does not crash -- it
// #  reports "no solution" for equations that do have one.
// ############################################################################
//
// PITFALLS:
//   The table must map value -> SMALLEST q (or be built with descending q) or
//     you get a valid x that is not the minimum. Below, later writes overwrite
//     earlier ones and q ascends, so the table holds the LARGEST q -- which is
//     what makes x = p*n - q smallest. Do not "fix" this.
//   x = 0 is a legal answer when b == 1. Check it before the loop.
//   All arithmetic needs __int128 or careful ll if m approaches 1e18.
//   m need not be prime. Nothing here assumes it.
//   Memory is O(sqrt m) hash entries; at m = 1e18 that is 1e9 -- BSGS is for
//     m up to about 1e12-1e14 in practice.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   a^x = b (mod m), find x                bsgs / bsgs_any
//   multiplicative ORDER of a mod m        smallest x > 0 with a^x = 1;
//                                          divides phi(m), so factor phi(m)
//                                          and test its divisors -- cheaper
//                                          than BSGS
//   primitive root mod p                   test g^((p-1)/q) != 1 for every
//                                          prime q | p-1
//   x^k = b (mod p)                        take a primitive root g, write
//                                          everything as a power of g, and it
//                                          becomes k*y = log_g(b) mod (p-1),
//                                          a linear congruence
//   how many steps until a cycle repeats    order, as above
//   "smallest t with f^t(x) == y" for an    same shape on a functional graph;
//     arbitrary function                    or just rho-detect with Floyd
//   Diffie-Hellman style toy crypto        the literal reading
//
// NOT THIS: if the modulus is small enough to walk (m <= 1e7), a plain loop is
//   shorter and faster. If you need the order, factor phi(m) instead.
// ============================================================================

// requires gcd(a, m) == 1. Returns the smallest x >= 0, or -1.
ll bsgs(ll a, ll b, ll m) {
    a %= m, b %= m;
    if (m == 1) return 0;
    if (b == 1) return 0;                        // a^0 = 1
    ll n = (ll)sqrtl((long double)m) + 1;
    unordered_map<ll, ll> tbl;
    ll cur = b % m;
    for (ll q = 0; q < n; q++) {                 // baby steps: b * a^q
        tbl[cur] = q;                            // later q overwrites -> largest q
        cur = (__int128)cur * a % m;
    }
    ll an = 1;
    for (ll i = 0; i < n; i++) an = (__int128)an * a % m;
    cur = 1;
    for (ll p = 1; p <= n + 1; p++) {            // giant steps: a^(p*n)
        cur = (__int128)cur * an % m;
        auto it = tbl.find(cur);
        if (it != tbl.end()) {
            ll x = p * n - it->second;
            if (x >= 0) return x;
        }
    }
    return -1;
}
// general m: no coprimality required
ll bsgs_any(ll a, ll b, ll m) {
    a %= m, b %= m;
    ll k = 1, add = 0, g;
    while ((g = __gcd(a, m)) > 1) {
        if (b == k) return add;                  // matched during the reduction
        if (b % g) return -1;                    // no solution
        b /= g, m /= g, add++;
        k = (__int128)k * (a / g) % m;
    }
    // now gcd(a, m) == 1; solve a^x * k = b  ->  a^x = b * k^-1
    ll n = (ll)sqrtl((long double)m) + 1;
    unordered_map<ll, ll> tbl;
    ll cur = b % m;
    for (ll q = 0; q < n; q++) {
        tbl[cur] = q;
        cur = (__int128)cur * a % m;
    }
    ll an = 1;
    for (ll i = 0; i < n; i++) an = (__int128)an * a % m;
    cur = k % m;
    for (ll p = 1; p <= n + 1; p++) {
        cur = (__int128)cur * an % m;
        auto it = tbl.find(cur);
        if (it != tbl.end()) {
            ll x = p * n - it->second + add;
            if (x >= add) return x;
        }
    }
    return -1;
}
