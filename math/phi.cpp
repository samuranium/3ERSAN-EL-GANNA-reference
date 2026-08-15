// ============================================================================
// EULER TOTIENT -- phi(n) = how many of 1..n are coprime with n
// ----------------------------------------------------------------------------
//   phi(n) = n * prod over distinct primes p | n of (1 - 1/p)
// Written as r -= r/p so it stays in integers -- never compute the fractions.
//
//   phi(p)     = p - 1          phi(p^k) = p^k - p^(k-1)
//   phi(a*b)   = phi(a)*phi(b)  when gcd(a,b) = 1      (multiplicative)
//   sum of phi(d) over d | n    = n                    (this is the useful one)
//
// ############################################################################
// #  WHY YOU ACTUALLY REACH FOR IT: EULER'S THEOREM
// #
// #      a^phi(m) = 1 (mod m)      whenever gcd(a, m) = 1
// #
// #  so a huge exponent reduces:  a^e = a^(e mod phi(m)) (mod m).
// #  Fermat is just this with m prime, where phi(m) = m-1.
// #
// #  THE TRAP: gcd(a, m) != 1 breaks it. The correct general rule is
// #      a^e = a^(e mod phi(m) + phi(m))  (mod m)   for e >= log2(m)
// #  Dropping the "+ phi(m)" is the classic wrong answer on power towers.
// ############################################################################
//
// PITFALLS:
//   phi(1) = 1 by convention. The loop below returns 1 -- correct.
//   The single-value version is O(sqrt n) and does trial division; for n up to
//     1e18 you need Pollard rho to factor first.
//   `r -= r / p` must run ONCE per DISTINCT prime -- that is what the inner
//     while loop guarantees by stripping p out completely first.
//   The sieve version is O(n log log n) and gives every phi up to n. Prefer it
//     whenever you need more than a handful of values.
//
// SUBPROBLEMS:
//   phi of one n                        phi()
//   phi of everything up to n           phi_sieve()
//   a^e mod m with astronomical e       Euler's theorem, see the box
//   count coprime pairs / fractions     sum of phi(i) -- Farey sequence length
//   sum over d|n of phi(d) = n          telescoping identity, used in
//                                       counting-by-gcd problems
//   count x in [1,n] coprime to m       inclusion-exclusion over m's distinct
//                                       primes, NOT phi (phi is x in [1,m])
//   multiplicative order of a mod m     divides phi(m) -- test the divisors
//   primitive root mod p                test g^((p-1)/q) != 1 for each prime
//                                       q | p-1
//   gcd-sum: sum of gcd(i, n) for i<=n  sum over d|n of d * phi(n/d)
//   DP/incexc.cpp uses the mu-sieve counterpart of phi_sieve
// ============================================================================

// O(sqrt n), by trial division
ll phi(ll n) {
    ll r = n;
    for (ll p = 2; p * p <= n; p++)
        if (n % p == 0) {
            while (n % p == 0) n /= p;    // strip it out: one factor per prime
            r -= r / p;
        }
    if (n > 1) r -= r / n;                // the leftover prime > sqrt
    return r;
}
// phi for every value in [0, n], O(n log log n)
void phi_sieve(int n, vector<int> &f) {
    f.resize(n + 1);
    iota(f.begin(), f.end(), 0);
    for (int i = 2; i <= n; i++)
        if (f[i] == i)                    // i is prime
            for (int j = i; j <= n; j += i) f[j] -= f[j] / i;
}
