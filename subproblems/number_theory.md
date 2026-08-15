# NUMBER THEORY — subproblems

**The code lives in the other team's `Number Theory.pdf`, not in this repo.**
Entries route to a section there. What we have locally is only
`number theory/sieve.cpp`, plus the modular tools in `math/`.

## PDF CONTENTS — what is in there

`Sieve and linear and segmented sieve` · `PHI` · `Mobius (function + inversion)`
`Miller-Rabin` · `Lucas` · `CRT` · `LDE and CRT` · `Diophantine Equations`
`Linear Modular Equation` · `Congruence` · `LDE with N variables`
`inverse any mod` · `Baby step Giant Step` · `Fast fib` · `floor division`
`Quadratic equation` · `Pascal` · `Stars and Bars` · `Catalan number`
`nCr any mod, n to 1e18` · `Counting` · `Number of Divisors of N`
`Count of pairs with lcm ≤ N` · `Count of pairs with gcd ≤ N` · `Rules` · `NOTES`

## BY QUESTION

| you see | do this | where |
|---|---|---|
| all primes up to n | sieve | `number theory/sieve.cpp`, PDF *Sieve* |
| primes up to 1e12, one range | segmented sieve | PDF *segmented sieve* |
| smallest prime factor / factor fast | linear sieve with SPF | PDF *linear sieve* |
| is this one number prime, n ≤ 1e18 | Miller–Rabin | PDF *Miller-Rabin* |
| factor a single big number | Pollard's rho | PDF |
| number of divisors of n | from the factorisation, Π(e+1) | PDF *Number of Divisors* |
| sum of divisors | Π (p^(e+1)−1)/(p−1) | PDF |
| how many ≤ n are coprime to n | φ(n) | `math/phi.cpp`, PDF *PHI* |
| φ for every value up to n | sieve version | `math/phi.cpp` |
| `a^e mod m` with astronomical e | Euler: e mod φ(m) **(+φ(m) if gcd≠1)** | `math/phi.cpp` |
| "gcd is exactly g" counting | Möbius / divisor inversion | `DP/incexc.cpp`, PDF *Mobius* |
| Σ over d\|n of something | divisor sum, invert | `DP/incexc.cpp` |
| count coprime pairs | Möbius, or φ sums | `DP/incexc.cpp` |
| `nCr mod p`, n up to 1e18 | Lucas | PDF *Lucas* |
| `nCr mod` a non-prime | factor the mod, CRT | PDF *nCr any mod* |
| is nCr odd | `(n & r) == r` | `math/bitwise.cpp` |
| `x ≡ r_i (mod m_i)` system | CRT | `math/crt.cpp`, PDF *CRT* |
| `ax + by = c` | extended gcd | `math/extgcd.cpp`, PDF *LDE* |
| `ax ≡ b (mod m)` | linear congruence | PDF *Linear Modular Equation* |
| `a^x ≡ b (mod m)` | BSGS | `math/bsgs.cpp`, PDF |
| n-th Fibonacci, huge n | fast doubling / matrix | PDF *Fast fib*, `DP/matrix/` |
| distribute n identical items into k boxes | stars and bars | PDF *Stars and Bars* |
| balanced brackets / triangulations / BSTs | Catalan | `combinatorics/catalan.cpp` |
| divisibility rule for k | PDF *Rules*, and `Misc.pdf` *divisablity Rules* |

## RECOGNITION

| the statement mentions | think |
|---|---|
| "coprime", "gcd = 1" | φ, Möbius |
| "exactly d" and "multiple of d is easy" | divisor inversion — `DP/incexc.cpp` |
| a huge exponent | Euler / Fermat reduction |
| a huge n in a binomial | Lucas |
| "how many pairs with gcd/lcm ..." | count multiples, invert |
| "the answer mod a composite" | CRT over prime powers |
| numbers up to 1e18, primality | Miller–Rabin, not a sieve |
| "period" / "order" / "cycle length" | multiplicative order, divides φ(m) |

## TRAPS

- Euler's theorem needs `gcd(a,m) = 1`. Without it use
  `a^e ≡ a^(e mod φ(m) + φ(m))` for `e ≥ log₂ m`. Dropping the `+φ(m)` is the
  classic wrong answer on power towers.
- Our `number theory/sieve.cpp` marks `[2, N)`; `prime[N]` itself is never set.
- Fermat inverse needs a **prime** modulus. Otherwise `math/extgcd.cpp`.
- `int` overflows in `i*i <= n` when n approaches 1e18 — use `ll` or compare
  `i <= n/i`.
