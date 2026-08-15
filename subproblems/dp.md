# DP — subproblems

## THE THREE QUESTIONS, IN ORDER

1. **What is the state?** Enough to decide the future, nothing more.
2. **What is the transition?** From state to state, and its cost.
3. **Is it too slow?** Only then reach for an optimisation below.

Most failures are step 1. If the transition is awkward, the state is wrong.

## CLASSIC SHAPES

| you see | state | file |
|---|---|---|
| items, capacity | dp[i][cap] | — |
| "is sum s reachable" (not how many) | bitset shifts | `DP/subset_sum_bitset.cpp` |
| n ≤ 20 items, subsets | dp[mask] | see BITMASK below |
| intervals, merge two halves | dp[l][r] | `DP/optimization/knuth.cpp` |
| a tree, answer per subtree | dp[v] over children | `DP/rerooting.cpp` |
| answer for **every** root | down[] + up[] | `DP/rerooting.cpp` |
| digits of a number, count ≤ N | dp[pos][tight][...] | `DP/digit dp.cpp` |
| digits + a divisibility/automaton rule | dp[pos][tight][autstate] | `DP/digit_dp_automaton.cpp` |
| tile a grid / cell-by-cell grid DP | broken profile, O(nm·2^m) | `DP/broken_profile_dp.cpp` |
| longest increasing subsequence | patience, O(n log n) | `DP/lis.cpp` |
| a DAG | topological order **is** the DP order | `graph/shortest path/dag paths.cpp` [5] |
| "at most k of something" | add k as a dimension | — |
| two sequences | dp[i][j] | — |
| a game between two players | dp[state] = win/lose | `game/grundy.cpp` |

## BITMASK (n ≤ 20–24)

| you see | do this |
|---|---|
| visit every vertex once (TSP) | dp[mask][last] — `DP/bitmask_tsp.cpp` |
| partition into groups | iterate submasks, O(3ⁿ) |
| "for each mask, aggregate over submasks" | SOS DP / zeta, O(2ⁿ·n) — `DP/sos.cpp` |
| ... and back again ("exactly" from "at least") | Mobius inverse, same loop, minus sign — `DP/sos.cpp` |
| count pairs with `a & b == 0` | SOS over the complement |
| assign n tasks to n people | dp[mask], the count of set bits is the row |
| choose exactly k | Gosper's hack — `math/bitwise.cpp` |
| n ≈ 40 | meet in the middle, 2^(n/2) |

Submask enumeration and all the bit primitives: `math/bitwise.cpp`.

## OPTIMISATIONS — only after the plain DP is written

| the recurrence looks like | tool | file |
|---|---|---|
| `dp[i] = min_j (m_j·x_i + c_j)` | CHT / Li Chao | `DP/optimization/CHT.cpp`, `li_chao.cpp` |
| ... and a line only applies on a range of x | Li Chao segments | `li_chao.cpp` |
| `dp[layer][i] = min_j dp[layer−1][j] + C(j,i)` | divide & conquer opt | `DP/optimization/D&C.cpp` |
| `dp[i][j] = min_k dp[i][k]+dp[k+1][j] + C(i,j)` | Knuth (needs QI) | `DP/optimization/knuth.cpp` |
| `dp[i] = min over a window of j` | monotonic deque | `DS/monotonic_deque.cpp` |
| a linear recurrence, huge n | matrix power | `DP/matrix/matrix expo.cpp` |
| min-plus over paths of exactly k edges | min-plus matrix power | `DP/matrix/minlenK.cpp` |
| "exactly k groups", cost convex in k | Lagrangian / aliens trick | their PDF |
| a convolution over sums | FFT/NTT | `math/fft_ntt.cpp` |
| counting with "exactly" constraints | inclusion–exclusion | `DP/incexc.cpp` |

## COUNTING WITH CONSTRAINTS

| you see | do this | file |
|---|---|---|
| "exactly d" where "multiple of d" is easy | divisor inversion / Möbius | `DP/incexc.cpp` |
| "gcd is exactly g" | count multiples, invert downward | `DP/incexc.cpp` |
| "at least one of each" | inclusion–exclusion over the missing set | `DP/incexc.cpp` |
| "no two adjacent" | dp with the last choice in the state | — |
| "avoid a set of patterns" | Aho automaton state in the DP | `strings/aho_corasick.cpp` |
| a subset xor equals x | xor basis, 2^(n−rank) | `DP/sub-problems xorbasis.cpp` |

## STATE-SPACE TRICKS

| you see | do this |
|---|---|
| the DP is over a graph of states | it is shortest path — Dijkstra/BFS on states |
| a parameter only ever increases | drop it, or make it the outer loop |
| the transition needs a range min | segment tree over the DP array |
| dp depends on dp[i−1] only | roll the array, O(n) memory |
| the answer is monotone in a parameter | binary search the parameter, DP the check |
| forward transitions are easier than backward | push instead of pull |

## TRAPS

- **Identity of the merge.** In rerooting, `up[root]` must be the identity of
  your operation — 0 for sum, −INF for max. Symptom: root correct, everything
  else wrong. `DP/rerooting.cpp`.
- **Knuth needs the quadrangle inequality.** No check, no warning; a wrong
  answer that looks plausible. `DP/optimization/knuth.cpp`.
- **D&C opt vs Knuth**: layered recurrence → D&C; interval split → Knuth. They
  are not interchangeable.
- **Digit DP array sizes.** Re-derive the state bounds per problem; a memo array
  smaller than the value range corrupts memory silently.
- **`min_plus` identity is not zero.** Diagonal 0, off-diagonal INF.
  `DP/matrix/minlenK.cpp`.
