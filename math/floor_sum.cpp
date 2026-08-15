// ============================================================================
// FLOOR SUM + DIVISOR BLOCKS -- the two "sum over floors" primitives
// ----------------------------------------------------------------------------
// [A] floor_sum(n, m, a, b) = sum_{i=0}^{n-1} floor((a*i + b) / m),  O(log)
//     A Euclid-like recursion: reduce a and b mod m, then SWAP the roles of
//     the two axes exactly like the gcd algorithm. Geometrically it counts
//     lattice points under the line y = (a*x + b)/m.
//
// [B] divisor blocks: floor(n/i) takes only O(sqrt n) DISTINCT values, and
//     each value occupies a contiguous block of i. The block containing i
//     ends at  n / (n / i).  So
//         for (ll i = 1, j; i <= n; i = j + 1) { j = n / (n / i); ... }
//     visits every block once, ~2*sqrt(n) iterations.
//
// ############################################################################
// #  floor_sum OVERFLOWS BEFORE IT EVER REDUCES
// #
// #  The term `a * n * (n-1) / 2` is ~1e36 for n = 1e9 and a = 1e9. It blows
// #  past 64 bits BEFORE any modulus is applied, so a `% MOD` at the end does
// #  not save you. Either keep every intermediate in __int128 (done below),
// #  or reduce a and b mod m up front -- which the first two lines do, and
// #  which is what keeps the recursion cheap as well.
// ############################################################################
//
// ############################################################################
// #  THE DIVISOR-BLOCK LOOP DIVIDES BY ZERO WHEN n == 0
// #
// #  `n / (n / i)` is a division by `n / i`, which is 0 as soon as i > n.
// #  With n = 0 the loop body runs once with i = 1 and crashes. Guard n >= 1
// #  before entering, every time -- it is the single most common way this
// #  loop fails, and it only fires on a degenerate test.
// #
// #  Same trap in the two-variable form: when iterating blocks of BOTH n/i
// #  and m/i, the block end is min(n/(n/i), m/(m/i)) and BOTH divisors must
// #  be non-zero -- clamp with `i > n ? LLONG_MAX : n/(n/i)`.
// ############################################################################
//
// PITFALLS:
//   floor_sum's i runs over [0, n-1], not [1, n]. Off by one here changes the
//     answer by floor(b/m) or floor((a*n+b)/m) depending on which end.
//   Negative a or b: the version below assumes a, b >= 0 and m > 0. For
//     negatives, shift b up by a multiple of m and correct afterwards.
//   `n / i` is INTEGER division throughout; writing it as a double is wrong
//     past 2^53.
//   The RETURN type is ll, and the internal accumulator is __int128. The
//     intermediates are safe, but if the true ANSWER exceeds 2^63 the return
//     truncates. n = 1e9 with a = 1e9 already does (the true value is ~5e26).
//     Change the return type to __int128, or take it mod p inside.
//   Same for divisor_block_weighted: sum sigma(i) is about 0.82*n^2, so it
//     leaves 64 bits somewhere past n = 3e9.
//   Divisor blocks give you the VALUE floor(n/i) and the block [i, j]. The
//     count of i in the block is j - i + 1 -- that factor is what most uses
//     multiply by.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   sum floor((a*i+b)/m) for i in [0,n)              floor_sum()
//   lattice points strictly under a line              same, adjust the boundary
//   lattice points inside a right triangle            same
//   count i in [1,n] with (a*i+b) mod m < c           floor_sum difference:
//                                                     F(c) - F(0) style
//   sum of i * floor(n/i)                             divisor blocks + the
//                                                     arithmetic series of i
//                                                     over each block
//   sum of floor(n/i) for i = 1..n                    divisor_block_sum()
//   number of divisors summed: sum_{i=1..n} d(i)      = sum floor(n/i) --
//                                                     the same call
//   sum_{i=1..n} sigma(i)                             = sum i*floor(n/i)
//   count pairs (i,j) with i*j <= n                   = sum floor(n/i)
//   "sum over i of f(floor(n/i))"                     divisor blocks -- the
//                                                     entry point to the Du
//                                                     and Min_25 sieves
//   sum_{i=1..n} floor(n/i) * floor(m/i)              two-variable blocks,
//                                                     divisor_block_pair()
//   sum of gcd / lcm over pairs                       Mobius over divisor
//                                                     blocks -- DP/incexc.cpp
//                                                     for the inversion
//
// NOT THIS:
//   you need the actual divisors of one n  -> trial division to sqrt n.
//   n is small (<= 1e6)                     -> a plain loop or a sieve is
//                                              clearer and just as fast.
// ============================================================================

// [A] sum_{i=0}^{n-1} floor((a*i + b) / m).  Requires n >= 0, m > 0, a,b >= 0.
// __int128 throughout: the quadratic term overflows 64 bits long before the end.
ll floor_sum(ll n, ll m, ll a, ll b) {
    __int128 ans = 0;
    while (true) {
        if (a >= m) {                             // strip the whole part of a
            ans += (__int128)(n - 1) * n / 2 * (a / m);
            a %= m;
        }
        if (b >= m) {                             // strip the whole part of b
            ans += (__int128)n * (b / m);
            b %= m;
        }
        __int128 ymax = (__int128)a * n + b;
        if (ymax < m) break;                      // the line never reaches y=1
        // swap the axes, exactly like the Euclidean algorithm
        n = (ll)(ymax / m);
        b = (ll)(ymax % m);
        swap(m, a);
    }
    return (ll)ans;
}

// [B] sum_{i=1}^{n} floor(n / i).  Also: the number of pairs (i,j) with i*j <= n,
// and sum_{i=1}^{n} d(i).
ll divisor_block_sum(ll n) {
    if (n <= 0) return 0;                         // the guard from the box
    ll res = 0;
    for (ll i = 1, j; i <= n; i = j + 1) {
        ll v = n / i;
        j = n / v;                                // last index with this value
        res += v * (j - i + 1);
    }
    return res;
}

// sum_{i=1}^{n} i * floor(n / i).  Equals sum_{i=1}^{n} sigma(i).
ll divisor_block_weighted(ll n) {
    if (n <= 0) return 0;
    ll res = 0;
    for (ll i = 1, j; i <= n; i = j + 1) {
        ll v = n / i;
        j = n / v;
        res += v * (i + j) * (j - i + 1) / 2;     // v * (sum of i over [i,j])
    }
    return res;
}

// sum_{i=1}^{min(n,m)} floor(n/i) * floor(m/i).  Blocks of BOTH, clamped.
ll divisor_block_pair(ll n, ll m) {
    ll lim = min(n, m);
    if (lim <= 0) return 0;
    ll res = 0;
    for (ll i = 1, j; i <= lim; i = j + 1) {
        ll vn = n / i, vm = m / i;
        j = min(n / vn, m / vm);                  // both divisors are non-zero
        j = min(j, lim);
        res += vn * vm * (j - i + 1);
    }
    return res;
}
