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
| derangements (no fixed point) | `D(n)=(n−1)(D(n−1)+D(n−2))` — `combinatorics/stirling.cpp` |
| surjections onto k labels | `k!·S2(n,k)` — `combinatorics/stirling.cpp` |
| "gcd exactly g" over a range | count multiples of g, invert downward |

## BURNSIDE / SYMMETRY

| you see | do this |
|---|---|
| count up to rotation | Burnside — `combinatorics/burnside.cpp` |
| necklaces with k colours, n beads | `(1/n) Σ_{d\|n} φ(d) k^(n/d)` — `combinatorics/burnside.cpp` |
| up to rotation **and** reflection | bracelets, parity-split reflections — `combinatorics/burnside.cpp` |
| count up to relabelling | Burnside over the explicit group — `combinatorics/burnside.cpp` |

## COMMON SEQUENCES

| name | meaning |
|---|---|
| Catalan | balanced structures — `combinatorics/catalan.cpp` |
| Stirling 2nd kind | partitions of n labelled items into k non-empty sets — `combinatorics/stirling.cpp` |
| Stirling 1st kind | permutations of n with exactly k cycles — `combinatorics/stirling.cpp` |
| Bell | total partitions of an n-set = Σ Stirling — `combinatorics/stirling.cpp` |
| Derangements | permutations with no fixed point — `combinatorics/stirling.cpp` |
| Fibonacci | tilings, no-two-adjacent choices |
| Narayana / Motzkin | Catalan relatives, see above |

## TRAPS

- Factorials must be precomputed once; recomputing per query is the usual TLE.
- Division is modular — multiply by the inverse. Prime modulus required.
- `nCr_without_mod.cpp` overflows `int` around n = 34.
- Off-by-one in Catalan: n triangles needs an (n+2)-gon. Check a tiny case.
- Inclusion–exclusion with 2^k terms needs k ≤ ~20.

---

# RESEARCH ADDITIONS


## COMBINATORICS

| you see | reach for | the tell |
|---|---|---|
| count integer partitions of n | **pentagonal number theorem** recurrence — `combinatorics/stirling.cpp` | generalised pentagonal indices k and -k, signs cycle + + - - |
| count labelled trees with degree constraints | Cayley / Prufer; degree-constrained is a multinomial over (d_i - 1) | Prufer turns tree counting into sequence counting |
| count **connected** labelled structures | EGF: `C = log(All)`, or the rooted-subtraction recurrence | "connected" is almost never counted directly |
| labelled objects, composition matters | **EGF** (divide by n!); "multiset of components" is exp, "extract components" is log | multinomials in the statement mean EGF |
| unlabelled sequences | **OGF**, a product of series, one FFT | — |
| count standard Young tableaux of a shape | **hook length formula**: `n! / product of hooks` | generalises ballot/Catalan to k candidates |
| lattice paths above a slope-k line, k-ary trees | **Fuss-Catalan** `(1/(kn+1)) C(kn+n, n)`; cycle lemma | the natural extension of Catalan |
| colourings up to rotation, unrestricted colour counts | Burnside, `c^cycles(g)` — `combinatorics/burnside.cpp` | the group is small and explicit |
| colourings up to symmetry with **fixed counts of each colour** | **Polya** cycle index — `combinatorics/burnside.cpp` `polya_fixed_counts` | Burnside alone cannot track colour multiplicities |
| convolution indexed by **gcd or lcm** | divisor / multiple zeta-Mobius transform, O(n log log n) | same machinery as SOS, different lattice |
| "exactly k of the properties hold" | inclusion-exclusion with `sum (-1)^(j-k) C(j,k) N_j` | different weights from "at least" |
| balls and boxes, unsure which formula | the **twelvefold way** table — `combinatorics/stirling.cpp` (it is in the banner) | fix the four labels first |
| sequences avoiding forbidden substrings | Aho automaton, transfer matrix, matrix power | "count strings of length N <= 1e18 avoiding S" |
