# COMBINATORICS — subproblems

## COUNTING BASICS

| you see | formula | file |
|---|---|---|
| choose k of n | `C(n,k)` | `combinatorics/nCr_with_mod.cpp` |
| arrange k of n | `n!/(n−k)!` | — |
| arrange n with repeats | `n! / Π(cᵢ!)` | — |
| distribute n identical into k boxes | `C(n+k−1, k−1)` (stars and bars) | their NT PDF |
| ... with each box ≥ 1 | `C(n−1, k−1)` | — |
| ... with box i ≤ uᵢ | inclusion–exclusion over violated bounds | `DP/incexc.cpp` |
| paths on a grid, only right/up | `C(w+h, w)` | — |
| ... avoiding a diagonal | reflection → Catalan / ballot | `combinatorics/catalan.cpp` |
| `C(n,k)` with n up to 1e18 | Lucas | their NT PDF |
| is `C(n,k)` odd | `(n & k) == k` | `math/bitwise.cpp` |

## CATALAN AND NEIGHBOURS

`combinatorics/catalan.cpp` — the file lists fourteen applications. Trigger:
**a sequence of +1/−1 whose every prefix sum stays ≥ 0**.

| you see | answer |
|---|---|
| balanced brackets, length 2n | `C(n)` |
| binary trees, n nodes | `C(n)` |
| triangulations of an (n+2)-gon | `C(n)` |
| stack-sortable permutations of 1..n | `C(n)` |
| non-crossing pairings of 2n points | `C(n)` |
| paths to (a,b) staying strictly ahead | ballot: `(a−b)/(a+b) · C(a+b,a)` |
| steps +1/−1/0, stay ≥ 0 | Motzkin |
| Dyck paths with exactly k peaks | Narayana |

## INCLUSION–EXCLUSION

`DP/incexc.cpp` — has the two dual forms worked out.

| you see | do this |
|---|---|
| "exactly d" and "multiple of d is easy" | invert the divisor sum |
| "at least one of each type" | subtract the subsets that miss a type |
| "none of these k bad properties" | 2^k signed terms, or Möbius |
| derangements (no fixed point) | `D(n) = n!·Σ(−1)^i/i!`, or `D(n)=(n−1)(D(n−1)+D(n−2))` |
| surjections onto k labels | `Σ(−1)^i C(k,i) (k−i)^n` |
| "gcd exactly g" over a range | count multiples of g, invert downward |

## BURNSIDE / SYMMETRY

| you see | do this |
|---|---|
| count up to rotation | Burnside: average number of fixed colourings over rotations |
| necklaces with k colours, n beads | `(1/n) Σ_{d\|n} φ(d) k^(n/d)` |
| up to rotation **and** reflection | add the reflection terms (n even and odd differ) |
| count up to relabelling | Burnside over the symmetry group |

## COMMON SEQUENCES

| name | meaning |
|---|---|
| Catalan | balanced structures — `combinatorics/catalan.cpp` |
| Stirling 2nd kind | partitions of n labelled items into k non-empty sets |
| Bell | total partitions of an n-set = Σ Stirling |
| Derangements | permutations with no fixed point |
| Fibonacci | tilings, no-two-adjacent choices |
| Narayana / Motzkin | Catalan relatives, see above |

## TRAPS

- Factorials must be precomputed once; recomputing per query is the usual TLE.
- Division is modular — multiply by the inverse. Prime modulus required.
- `nCr_without_mod.cpp` overflows `int` around n = 34.
- Off-by-one in Catalan: n triangles needs an (n+2)-gon. Check a tiny case.
- Inclusion–exclusion with 2^k terms needs k ≤ ~20.
