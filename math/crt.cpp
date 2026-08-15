// ============================================================================
// CHINESE REMAINDER THEOREM -- merge two congruences, NON-coprime moduli OK
// ----------------------------------------------------------------------------
// Given  x = r1 (mod m1)  and  x = r2 (mod m2),  produce the single congruence
//     x = r (mod lcm(m1, m2))
// that is equivalent to both, or report that none exists.
//
// ############################################################################
// #  MOST WRITE-UPS ASSUME COPRIME MODULI. THIS ONE DOES NOT.
// #
// #  With gcd(m1, m2) = g, a solution exists IFF  (r2 - r1) % g == 0.
// #  Coprime moduli have g = 1, so that test is always true and the failure
// #  case disappears -- which is why the textbook version never mentions it.
// #  Contest inputs are frequently NOT coprime (x = 1 mod 4, x = 3 mod 6 has
// #  no solution). Always check the returned m for -1.
// ############################################################################
//
// FOLDING MANY CONGRUENCES: merge pairwise, left to right. Any failure makes
//   the whole system unsolvable, so bail on the first -1.
//       pair<ll,ll> cur = {r[0], m[0]};
//       for (i = 1..k-1) { cur = crt(cur.first, cur.second, r[i], m[i]);
//                          if (cur.second == -1) break; }
//
// ############################################################################
// #  OVERFLOW IS THE REAL DANGER HERE
// #
// #  lcm grows as you fold. Ten moduli near 1e9 gives an lcm around 1e90 --
// #  it does NOT fit and the answer is meaningless long before you notice.
// #  The intermediate t * m1 overflows even when the final lcm fits, which is
// #  why it goes through __int128 below.
// #  If the product of all moduli can exceed ~9e18, you need big integers or
// #  a different formulation. Check the bound BEFORE coding.
// ############################################################################
//
// PITFALLS:
//   Requires extgcd from math/extgcd.cpp.
//   Inputs must satisfy 0 <= r < m. Normalise first: r = ((r % m) + m) % m.
//   m1 or m2 equal to 1 is legal and absorbs into the other.
//   The result r is the SMALLEST non-negative solution; the full set is
//     r + k*lcm.
//
// SUBPROBLEMS:
//   merge two congruences               crt()
//   solve a system of k congruences     fold pairwise, see above
//   count x in [1, N] with x = r mod m  (N - r) / m + 1 after folding
//   huge exponent by factoring the mod  compute mod each prime power, then CRT
//   Garner / reconstructing a big value from residues -- same machinery, and
//     the reason NTT uses several primes and CRTs the results back
// ============================================================================

// needs extgcd() from math/extgcd.cpp
ll extgcd(ll a, ll b, ll &x, ll &y);

// x = r1 (mod m1), x = r2 (mod m2)  ->  {r, lcm}, or {-1, -1} if impossible
pair<ll, ll> crt(ll r1, ll m1, ll r2, ll m2) {
    ll x, y, g = extgcd(m1, m2, x, y);
    if ((r2 - r1) % g) return {-1, -1};          // see the box
    ll lcm = m1 / g * m2;
    ll md = m2 / g;
    ll t = ((__int128)((r2 - r1) / g) % md) * x % md;
    ll r = (ll)(((__int128)t * m1 + r1) % lcm);
    return {(r % lcm + lcm) % lcm, lcm};
}
