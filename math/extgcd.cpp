// ============================================================================
// EXTENDED EUCLID -- gcd plus the coefficients, O(log min(a,b))
// ----------------------------------------------------------------------------
// extgcd(a, b, x, y) returns g = gcd(a, b) and fills x, y with a solution of
//     a*x + b*y = g          (Bezout's identity)
// Everything below is that one fact, applied.
//
// ############################################################################
// #  THIS IS THE INVERSE THAT WORKS WHEN FERMAT DOES NOT
// #
// #  math/modular.cpp divi() needs a PRIME modulus. inv_mod() here needs only
// #  gcd(a, m) == 1, so it covers mod 1e9, mod 2^32, mod a product of primes.
// #  And when the inverse does not exist it RETURNS -1 instead of silently
// #  producing a wrong number -- always check the return.
// ############################################################################
//
// PITFALLS:
//   x and y can be NEGATIVE and can exceed the inputs. Normalise before using
//     as a residue: ((x % m) + m) % m.
//   inv_mod normalises a first -- passing a negative a directly is otherwise
//     undefined.
//   diophantine() returns ONE solution. The full family is
//         x + k*(b/g),  y - k*(a/g)   for any integer k
//     which is what you need for "smallest positive x" style questions.
//   Overflow: x*(c/g) can be large. Use __int128 if a, b, c approach 1e18.
//   extgcd is recursive, depth O(log) -- fine.
//
// SUBPROBLEMS:
//   modular inverse, any modulus       inv_mod(a, m)
//   solve a*x + b*y = c over integers  diophantine()
//   smallest non-negative x with       shift by k*(b/g) -- see the note above
//     a*x = c (mod b)
//   linear congruence a*x = c (mod m)  x = c/g * inv(a/g, m/g), g solutions
//   CRT with non-coprime moduli        math/crt.cpp, built on this
//   rational reconstruction            extgcd on (mod, value), stop when the
//                                      remainder drops below sqrt(mod)
//   count lattice points on a segment  gcd(dx, dy) + 1 -- geometry, but the
//                                      same gcd
// ============================================================================

// returns g = gcd(a, b); sets x, y with a*x + b*y = g
ll extgcd(ll a, ll b, ll &x, ll &y) {
    if (!b) return x = 1, y = 0, a;
    ll x1, y1, g = extgcd(b, a % b, x1, y1);
    x = y1, y = x1 - (a / b) * y1;
    return g;
}
// a^-1 mod m for ANY m coprime with a. Returns -1 when it does not exist.
ll inv_mod(ll a, ll m) {
    ll x, y;
    a = ((a % m) + m) % m;
    if (extgcd(a, m, x, y) != 1) return -1;
    return ((x % m) + m) % m;
}
// one solution of a*x + b*y = c. false if c is not a multiple of gcd(a,b).
// g receives the gcd; the general solution is x + k*(b/g), y - k*(a/g).
bool diophantine(ll a, ll b, ll c, ll &x, ll &y, ll &g) {
    g = extgcd(llabs(a), llabs(b), x, y);
    if (c % g) return false;
    x *= c / g, y *= c / g;
    if (a < 0) x = -x;
    if (b < 0) y = -y;
    return true;
}
