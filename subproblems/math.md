# MATH — subproblems

Number-theory specific questions (primes, divisors, factorisation) are in
`number_theory.md` and route to the other team's PDF.

## MODULAR

| you see | do this | file |
|---|---|---|
| `a^e mod p` | fast power | `math/modular.cpp` |
| `1/a mod p`, p prime | `fp(a, p−2)` | `math/modular.cpp` |
| `1/a mod m`, any m | extended gcd, reports failure | `math/extgcd.cpp` |
| every inverse 1..n | O(n) table | `math/modular.cpp` |
| mod > 2³¹ | `__int128` mulmod | `math/modular.cpp` |
| huge exponent, `gcd(a,m)=1` | reduce e mod φ(m) | `math/phi.cpp` |
| huge exponent, gcd ≠ 1 | e mod φ(m) **+ φ(m)** | `math/phi.cpp` |
| `a^x = b mod m`, find x | baby-step giant-step | `math/bsgs.cpp` |
| `ax + by = c` over integers | diophantine | `math/extgcd.cpp` |
| `x ≡ r1 (m1)`, `x ≡ r2 (m2)` | CRT, non-coprime safe | `math/crt.cpp` |
| answer mod a non-prime | factor it, CRT the parts | `math/crt.cpp` |

## LINEAR ALGEBRA

| you see | do this | file |
|---|---|---|
| solve a linear system mod p | Gauss | `math/gauss.cpp` |
| count solutions | p^(free variables) | `math/gauss.cpp` |
| rank of a matrix | pivots | `math/gauss.cpp` |
| determinant | `det_mod` | `math/gauss.cpp` |
| matrix inverse | `inverse_mod` | `math/gauss.cpp` |
| parity / xor constraints | GF(2) Gauss with bitsets | `math/gauss.cpp` |
| max xor of a subset | xor basis | `DP/sub-problems xorbasis.cpp` |
| "every X an even number of times" | parity vectors + xor basis | `DP/sub-problems xorbasis.cpp` |
| number of spanning trees | Kirchhoff = a determinant | `graph/kirchhoff.cpp` |
| linear recurrence, huge n | matrix power | `DP/matrix/matrix expo.cpp` |
| probabilities / expected values with cycles | Gauss over the reals | `math/gauss.cpp` |
| random walk hitting time | same | `math/gauss.cpp` |

## POLYNOMIALS / CONVOLUTION

| you see | do this | file |
|---|---|---|
| multiply polynomials | NTT (998244353) | `math/fft_ntt.cpp` |
| ... modulo 1e9+7 or ANY modulus | 3 NTTs + Garner CRT | `math/convolution_anymod.cpp` |
| ... real or floating data | FFT | `math/fft_ntt.cpp` |
| count pairs by SUM | convolution | `math/fft_ntt.cpp` |
| count pairs by XOR / AND / OR | FWHT, **not** FFT | `math/fwht.cpp` |
| big integer multiplication | FFT on digits, then carry | `math/fft_ntt.cpp` |
| string matching with wildcards | 3 convolutions | `math/fft_ntt.cpp` |
| polynomial inverse / exp / division | Newton on top of NTT | their Math PDF |

## RANDOMIZED

| you see | do this | file |
|---|---|---|
| verify `A*B == C` without the product | Freivalds, O(n²) | `math/randomization.cpp` |
| an adversarial worst case (sort, hash) | shuffle first | `math/randomization.cpp` |
| `unordered_map` being hacked | splitmix64 custom hash | `math/randomization.cpp` |
| "a value covering > half the range" | sample 30 positions | `math/randomization.cpp` |
| need a unique minimum | random tie-break (isolation lemma) | `math/randomization.cpp` |
| NP-hard with a generous limit | random restarts / annealing | `math/randomization.cpp` |
| generating tests | `stress/gen.cpp` |

## BITS

All of it: `math/bitwise.cpp`, and `DS/bitset.cpp` for the ÷64 loop shapes.

| you see | do this |
|---|---|
| lowest set bit | `x & -x` |
| clear lowest set bit | `x & (x−1)` |
| index of the highest bit | `__lg(x)` |
| count set bits | `__builtin_popcountll` |
| iterate submasks of a mask | `for (s = m; ; s = (s−1) & m)` |
| iterate supermasks | `for (s = m; s < 1<<n; s = (s+1) \| m)` |
| all k-subsets | Gosper's hack |
| xor of 1..n | closed form by `n mod 4` |
| is nCr odd | `(n & r) == r` |
| a + b without `+` | `(a^b) + 2(a&b)` |
| gray code | `i ^ (i>>1)` |

## TRAPS

- `1 << 63` is UB. `1LL << 63`.
- `__builtin_ctz(0)` and `__builtin_clz(0)` are undefined.
- `a & 1 == 0` parses as `a & (1 == 0)`.
- There is **no NTT modulo 1e9+7**. 998244353 = 119·2²³ + 1.
- FFT precision: `n · max|a| · max|b|` must stay under ~9e15.
- `divi()` needs a **prime** modulus; use `inv_mod` otherwise.
- Catalan/binomial division is modular — multiply by the inverse, never `/`.

---

# RESEARCH ADDITIONS


## SEQUENCES, RECURRENCES, POLYNOMIALS

| you see | reach for | the tell |
|---|---|---|
| a sequence you can brute-force but need term k ~ 1e18 | **Berlekamp-Massey** then Kitamasa — `math/linear_recurrence.cpp` | any DP with a fixed finite state and linear transitions IS a linear recurrence, even unnamed |
| a known recurrence of order d, huge k | Kitamasa O(d^2 log k) — `math/linear_recurrence.cpp`; Bostan-Mori O(d log d log k) | beats matrix power O(d^3 log k) once d is about 60 |
| matrix power with big d but you need one entry | Cayley-Hamilton: reduce `x^k mod charpoly` | d^3 log k becomes d^2 log k |
| `[x^n]` of a rational function P/Q | Bostan-Mori directly | a rational GF is exactly a linear recurrence — skip BM |
| the answer looks like a polynomial in n of degree <= d | **Lagrange interpolation** on consecutive points, O(d) — `math/interpolation.cpp` | confirm by finite differences flattening to a constant |
| `sum i^k` for n up to 1e18 | interpolate degree k+1 — `math/interpolation.cpp` `sum_powers` | the prefix sum of a degree-k polynomial is degree k+1 |
| 1/A, log A, exp A, sqrt A, A^k mod x^n | Newton iteration, all O(n log n) | exp/log are the combinatorial workhorses |
| evaluate one polynomial at n points | multipoint evaluation, O(n log^2 n) via a product tree | the inverse direction is fast interpolation |
| evaluate at a geometric progression, or convolve at an awkward length | Chirp-Z / Bluestein | turns evaluation into one convolution |
| `f[n]` defined by a convolution of f with itself | relaxed / online (D&C) convolution, O(n log^2 n) | plain FFT is circular here |
| a product of many small polynomials | D&C, or a priority queue merging by degree | O(n log^2 n) vs O(n^2) |
| given power sums, want elementary symmetric (or reverse) | Newton's identities | "sum of a_i^k" against "sum over subsets of products" |
| non-crossing families of paths | **Lindstrom-Gessel-Viennot** determinant — `combinatorics/counting_formulas.cpp` | count of non-intersecting path systems = det of single-path counts |
| count Eulerian circuits in a digraph | **BEST theorem** — `combinatorics/counting_formulas.cpp` | pairs with the Kirchhoff you already have |
| permanent / "one per row and column", n <= 20 | Ryser — `combinatorics/counting_formulas.cpp` | determinant only works if signs are allowed |

## PROBABILITY AND EXPECTATION

| you see | reach for | the tell |
|---|---|---|
| "expected number of X" | linearity + one indicator per candidate | independence is NOT required — that is why it always works |
| E[max], E[#distinct], "at least one" | `E[X] = sum_{t>=1} P(X >= t)` | converts a max into a sum of tail probabilities |
| expected steps, transitions form a cycle | linear system + Gauss, or an absorbing chain `(I-Q)^-1` | memoised DP infinitely recurses here |
| column sums of `(I-Q)^-1` | expected steps to absorption from each transient state | also gives expected visits per state |
| a random walk on a graph, hitting / commute times | effective resistance; commute time = `2|E| R_eff` | — |
| "retry until success" | `E = 1 + p_fail * E`, so `E = 1/p_success` | collapses an infinite series to one line |

## TRANSFORMATIONS

| shape | actually is | note |
|---|---|---|
| "find a_k, k up to 1e18, no formula given" | generate 2d+ terms, Berlekamp-Massey, Kitamasa — `math/linear_recurrence.cpp` | any bounded-state linear DP is linear-recurrent |
| "the answer behaves smoothly in n" | a polynomial in n, Lagrange interpolation — `math/interpolation.cpp` | verify: finite differences become constant |
| "count lattice points under a line or inside a triangle" | `sum floor((ai+b)/m)` — `math/floor_sum.cpp` | also handles the `i * floor(...)` and squared variants |
| "sum over i of something depending on n/i" | O(sqrt n) divisor blocks — `math/floor_sum.cpp` | the entry point to Du and Min_25 sieves |
| "sum over pairs of gcd or lcm" | Mobius / divisor-lattice convolution | the divisor lattice is the subset lattice with primes as bits |
| `max or min of abs(x_i-x_j) + abs(y_i-y_j)` | 2^d sign enumeration: `abs(a)+abs(b) = max(+-a +- b)` | generalises Manhattan-Chebyshev to d dimensions |
| diagonal-only constraints (bishops) | rotate 45 degrees into two independent rook problems | black and white diagonals never interact |
| "choose exactly k, cost convex in k" | aliens trick: binary search the penalty, drop the k dimension | requires convexity of cost-vs-k |
| "distinct up to rotation/reflection" | Burnside / Polya — `combinatorics/burnside.cpp` | the group is tiny — enumerate it |
| "count connected X" | count all X then `log` the EGF | never count connected directly |
| "number of walks of length k" | matrix power; huge k with structured M means charpoly + Bostan-Mori | the transfer matrix is the state automaton's adjacency |
| "does a + b = c exist across two sets" | convolution (FFT), bitset shift, or Minkowski sum | one identity, three data types |
| "are these two multisets equal" | Zobrist XOR hashing with random 64-bit values | O(1) comparison and O(1) incremental update |
| "count subarrays with property P" | prefix-transform until P becomes an equality of two prefix values | XOR-prefix, sum-prefix, count-of-a minus count-of-b |
| "grid paths avoiding k obstacles" | sort obstacles, DP over "first obstacle hit" with inclusion-exclusion | O(k^2) regardless of grid size |
| "non-crossing tuples of paths / plane partitions" | LGV or hook length — `combinatorics/counting_formulas.cpp` | — |
| "piecewise-linear convex cost, add/shift/clamp repeatedly" | slope trick — `DP/slope_trick.cpp` | each operation is a heap push |
| "probability where states revisit each other" | linear system + Gauss, not memoisation | the cycle is the tell |
| "closest fraction with bounded denominator" | continued fractions / Stern-Brocot | convergents plus semiconvergents |
| "constraints on base-p digits of n and r" | Lucas (mod p) / Kummer (divisibility) | digits are the native language of these theorems |

## TRAPS

- **Berlekamp-Massey with only 2d-1 terms** finds a plausible but wrong recurrence that passes samples. Feed at least 2d.
- **Lagrange interpolation with the wrong degree**: `sum i^k` is degree **k+1**. One point short gives silent garbage.
- **Interpolation when p <= degree**: the denominators hit 0 mod p. Only valid for p > d.
- **nCr mod a composite via inverse factorials**: inverses of factorials do not exist mod p^e. Strip p, use generalised Lucas per prime power, CRT.
- **`a^((p+1)/4)` as a blanket square root** returns a value even for non-residues. Test the Legendre symbol first.
- **floor_sum overflow**: `a*n*(n-1)/2` blows past 64 bits before any reduction. Reduce a and b mod m up front, use `__int128`.
- **Divisor-block loop** divides by zero or loops forever when n = 0.
- **Du sieve / Min_25 memoised by index**: values are indexed by `floor(n/i)`, not i; two indices collapse near sqrt n. Use two arrays with the boundary right.
- **Pentagonal recurrence signs** cycle + + - -, and BOTH k and -k generalised pentagonal numbers are needed.
- **Burnside with the wrong group**: rotations only is cyclic (n elements); rotations plus reflections is dihedral (2n), and odd vs even n have different reflection cycle structures. Include the identity.
- **Burnside with fixed colour counts**: `c^cycles` is only valid when every colour is unlimited. Use the cycle index polynomial.
- **Grundy XOR applied to a scoring game**: Grundy encodes only who makes the last move.
- **Misere handled by "flip the answer"**: the all-piles-<=1 rule is specific to Nim and does not generalise.
- **Sprague-Grundy on a partizan game** breaks the theorem entirely.
- **Convex hull with duplicates or all-collinear input**: dedupe first, and decide explicitly whether boundary-collinear points are kept — calipers usually want them out.
- **Cross products**: always subtract the pivot FIRST; use `__int128` for triple products.
- **Sorting by atan2** is slow, lossy, and inconsistent at equal angles. Use `half(p)` + cross product, all integer.
- **Shoelace divided too early**: accumulate 2A (an exact integer), divide once at the end.
- **Minkowski sum on unnormalised polygons**: requires CCW order starting at the bottom-most then left-most vertex, with no collinear consecutive edges.
- **Integer ternary search** with `r - l < 3` cannot pick two distinct midpoints and loops forever. Loop while `r - l > 2`, then brute-force.
- **Ternary search on a non-strictly-unimodal function**: flat regions make `f(m1) == f(m2)` ambiguous. Binary search on the sign of `f(m+1) - f(m)` on integral domains.
- **Aliens trick with ties**: several k achieve the same optimum. Track the min/max achievable count and verify convexity.
- **LGV returns a SIGNED sum** over permutations; it counts non-crossing paths only when the identity permutation is the sole non-crossing connection.
- **Modular expected value with a non-invertible denominator** almost always means the model is wrong, not the problem.
- **Zobrist XOR on a MULTISET**: two equal elements cancel. Use per-occurrence random values or a sum hash.
- **Adaptive Simpson without a depth cap** recurses forever on discontinuities. Split at known breakpoints first.
- **Frobenius `ab-a-b`** is valid only for exactly two coprime coins.
- **Pell by brute force**: for D=61 the fundamental x is about 1.77e9.
