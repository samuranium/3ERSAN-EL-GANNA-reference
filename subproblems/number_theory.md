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

---

# RESEARCH ADDITIONS


## NUMBER THEORY

| you see | reach for | the tell |
|---|---|---|
| `sum floor((a*i+b)/m)` for i in [0,n) | **floor_sum**, Euclidean-like recursion, O(log) | also: lattice points under a line, points in a right triangle |
| `sum f(floor(n/i))` over i = 1..n | **divisor-block loop** `i = n/(n/i) + 1` | harmonic lemma: only ~2 sqrt n distinct quotients |
| prefix sum of phi, mu, d, sigma for n up to 1e10 | **Dirichlet hyperbola / Du sieve**, O(n^(2/3)) | "sum over i=1..N of a multiplicative function", N far past the sieve range |
| pi(n) or the sum of primes <= n, n up to 1e13 | **Lucy_Hedgehog** DP, O(n^(3/4)) time, O(sqrt n) memory | "count or sum primes" with n far past sieving |
| the sum of an arbitrary multiplicative f | **Min_25 sieve** | needs f(p^k) computable fast |
| any `sum over d|n of f(d) g(n/d)` | Dirichlet convolution; O(n log n), O(n) if multiplicative | sits under the Mobius inversion you have |
| `x^2 = a (mod p)` | **Tonelli-Shanks** (shortcut `a^((p+1)/4)` when p = 3 mod 4); Legendre symbol to test first | also the constant-term step inside polynomial sqrt |
| `x^k = a (mod p)` | discrete root: primitive root + BSGS; exactly gcd(k, p-1) solutions | reduces to a discrete log in the exponent |
| nCr mod a **composite** or mod p^e | factor the modulus, generalised Lucas per prime power, recombine by CRT | inverse factorials do not exist mod p^e — you must strip p's |
| "is nCr divisible by p^k" | **Kummer**: the exponent equals the number of carries when adding r and n-r in base p | Legendre: `v_p(n!) = (n - s_p(n))/(p-1)` |
| the largest power of p in n! | Legendre / digit-sum formula | needed for nCr mod p^e and trailing zeros |
| best rational p/q approximating x with q <= Q | continued-fraction convergents **plus semiconvergents** | convergents alone miss the optimum on the boundary |
| the fraction in an interval with the smallest denominator | **Stern-Brocot** descent with run-length jumps, O(log) | naive single steps are O(p+q) |
| `x^2 - D y^2 = 1` | **Pell**: fundamental solution from the CF period of sqrt(D), then a recurrence | the fundamental solution can be astronomically large |
| the largest amount not payable with given coins | **Frobenius**: `ab-a-b` for two coprime coins; 3+ coins is a shortest path over residues mod the smallest | no closed form beyond two |
| sums of two squares, or complex integer arithmetic | Gaussian integers; the norm is multiplicative | "a^2 + b^2 = n", or rotations by 90 degrees |
| distinct non-consecutive Fibonacci decomposition | **Zeckendorf** | also the exact loss condition in Fibonacci Nim |
| people in a circle, every k-th eliminated | **Josephus**: `J(n,k) = (J(n-1,k)+k) mod n`; O(k log n) for small k | repeated elimination with wraparound |
| the cycle length of `x -> a*x mod m` | multiplicative order: it divides phi(m); factor phi(m) and test divisors | — |
| enumerate Pythagorean triples | `(m^2-n^2, 2mn, m^2+n^2)`, m > n, coprime, opposite parity, scaled by k | every triple exactly once |
