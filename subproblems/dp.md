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
| partition into groups | iterate submasks, O(3ⁿ) — `DP/submask_partition.cpp` |
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


---

# RESEARCH ADDITIONS

## PROFILE / CONNECTIVITY DP (beyond tilings)

| you see | reach for | the tell |
|---|---|---|
| grid, narrow side W <= 12, answer depends on **which cells are joined**, not just filled | **connection-profile DP**: each boundary plug carries a connectivity label, valid states are bracket sequences | occupancy alone cannot distinguish one region from two |
| "exactly one closed loop", "single cycle covering all cells" | plug DP; merging two plugs of the **same** label is legal only at the final cell | early closure means multiple loops |
| Hamiltonian path (not cycle) in a grid | plug DP + a dimension for the number of unmatched endpoints (0/1/2) | endpoints are plugs with no partner |
| plug-DP state count nominally huge, most states illegal | hash map over the **minimum representation** (relabel connectivity classes to lexicographically smallest) | legal states are exponentially sparse; canonicalisation collapses aliases |
| grid cells have k > 2 states | profile as a **base-k number**, map-backed if k^W is loose | bitmask generalises to a k-ary digit per column |
| connect <= 15 terminals in a weighted graph | **Steiner tree DP** `d[v][S]` — `graph/steiner_tree.cpp` | "minimum network joining these k nodes" |

## SUBSET DP BEYOND SOS

| you see | reach for | the tell |
|---|---|---|
| `h(S) = sum over DISJOINT A + B = S of f(A)g(B)` | **subset-sum convolution**: add a popcount-rank dimension, zeta each rank, convolve ranks, inverse zeta. O(2^n n^2) | plain OR-convolution over-counts overlapping pairs; the rank enforces disjointness |
| convolution indexed by OR / AND | zeta over subsets / supersets, pointwise multiply, Mobius back. O(2^n n) | — |
| "at least these bits set", counts over supersets | **superset zeta** — invert the BIT TEST in the SOS loop, not the mask loop | — |
| partition n <= 22 items with arbitrary group cost | subset-sum convolution / set-power-series exp, O(2^n n^2) | beats 3^n once n is about 20 |
| you enumerate submasks but want **unordered** partitions | force the lowest set bit into one side | otherwise every partition is counted twice |

## DIGIT-LIKE DP

| you see | reach for | the tell |
|---|---|---|
| a predicate over a **pair or triple** of numbers, each with its own bound | one `tight` flag **per bound** | independent limits do not collapse into one |
| the condition uses AND / OR / XOR against a limit | the same machinery in **base 2**, MSB to LSB, per-operand "already strictly less" flags | digit DP where the digits are bits |
| the bound is given in base b | convert the bound to base b, digit DP over those digits | — |
| the **sum** of f over the range, not the count | carry (count, sum); the sum transition needs `count * digit * b^pos` | a lone accumulator loses the positional weight |
| "find the k-th number satisfying ..." | binary search the answer, digit DP as the counting oracle | counting is easy, constructing is not |

## PROBABILITY, EXPECTATION, GAMES

| you see | reach for | the tell |
|---|---|---|
| expected value where transitions **revisit** states | build the linear system, Gauss O(V^3) | the state graph is not a DAG, so no evaluation order exists |
| the same, but the state graph is a tree or line | back-substitution in O(V) | structure beats O(V^3) |
| a state transitions to itself with probability p | solve algebraically: `E = rest / (1 - p)` | a wasted-turn self-loop needs no elimination |
| "expected number of steps until ..." | linearity over indicators, or the tail sum `sum P(not done by t)` | often removes the DP entirely |
| variance / E[X^2] | carry (E[X], E[X^2]); the square transition needs the cross term `2 E[X] delta` | E[X^2] does not compose like E[X] |
| two players each maximising **their own** score | minimax DP storing (my total - their total) as ONE value | collapses two arrays into one |
| a game where moves may return to a prior position, or draws exist | **retrograde BFS** — `game/retrograde.cpp` | memoised win/lose DFS loops forever or mislabels draws as losses |
| a move **splits** the game into pieces | Grundy of the split = XOR of the pieces | — |

## OPTIMISATION AND CONVEXITY

| you see | reach for | the tell |
|---|---|---|
| `dp[i] = min_j dp[j] + w(j,i)`, w has QI, and the transition **uses dp values** | **1D/1D optimisation**: deque of candidate intervals + binary search, O(n log n) | D&C-opt needs offline layers; this is online |
| row minima of an implicitly defined **totally monotone** matrix | **SMAWK**, O(n+m); online version is LARSCH | D&C only needs monotone; SMAWK needs TM but is linear |
| (max,+) convolution where **one** array is convex | SMAWK, O(n) | the standard knapsack-layer speed-up |
| (max,+) convolution where **both** are convex | merge their difference sequences (Minkowski sum of epigraphs), O(n+m) | the slopes just get sorted |
| lines get **mutated** (`b_i += t*a_i` on a range), queries non-monotone | kinetic segment tree | Li Chao wants static lines; CHT wants monotone queries |
| piecewise-linear convex cost; transitions are `+abs(x-a)`, `+max(0,x-a)`, prefix-min, shift | **slope trick** — `DP/slope_trick.cpp` | cost shaped like distance-to-a-target |
| the same, merged up a tree | slope trick (`DP/slope_trick.cpp`) + small-to-large heap merging | — |
| convex cost, "move units between positions" | min-cost flow modelled as a **greedy with a regret heap** (push an undo option when you commit) | successive shortest paths collapses to a priority queue |
| the answer is a polynomial in n of degree <= d, n huge | brute d+1 values, **Lagrange interpolation** — `math/interpolation.cpp` | — |
| you suspect a linear recurrence but cannot derive it | brute ~2d terms, **Berlekamp-Massey** then Kitamasa — `math/linear_recurrence.cpp` | O(d^2 log n), no matrix cube |
| `dp[n]` depends on a convolution **containing dp itself** | **relaxed / online convolution** (CDQ + NTT), O(n log^2 n) | you cannot FFT what you have not computed |
| counting "sets of connected pieces", recurrence full of binomials | **EGF**: `exp(C)` = sets of C, `log` inverts | — |

## TREES

| you see | reach for | the tell |
|---|---|---|
| merge children arrays indexed by "#chosen in subtree" | tree knapsack, loops capped at `min(k, size)` — `DP/tree_knapsack.cpp` | each pair is charged once at its LCA — but only with the caps |
| tree knapsack with a numeric capacity W | flatten to DFS order, **take / skip-whole-subtree (jump to tout)**, O(n*W) — `DP/tree_knapsack.cpp` | a 1D knapsack with jumps, no child merging |
| DP arrays indexed by **depth**, merged up the tree | **long-path decomposition** with shared arrays + pointer offsets, O(n) total | each vertex is copied only at the top of its long path |
| DP arrays indexed by **value**, merged up the tree | segment tree merging — `DS/segment tree/merge/segtree_merge.cpp` | amortised by nodes destroyed during merge |
| vertex weights point-updated, reprint the whole-tree DP each time | **dynamic DP**: transition as a (max,+) matrix, chain matrices in a segment tree over HLD, O(log^2 n) per update | recompute-from-scratch is O(n) per query |

## OFFLINE / DYNAMIC DP

| you see | reach for | the tell |
|---|---|---|
| "answer for the array with element i removed", for every i | **D&C over indices** carrying the DP of everything OUTSIDE the interval, O(n log n) | division / undo is unavailable for max-type DP |
| items appear and disappear over time, DP answered at each moment | segment tree over time + a rollback-able structure | the offline-dynamic-connectivity pattern applied to DP |
| offline queries each asking the DP over an arbitrary subarray | **D&C by queries**: DP over suffixes of the left half and prefixes of the right, answer only straddling queries | — |

## STATE-SPACE AND REPRESENTATION

| you see | reach for | the tell |
|---|---|---|
| one parameter huge, but the DP **value** is small | **swap value and parameter**: index by value, store the minimal parameter | weights <= 1e9 but total value <= 1e5 |
| two parameters only ever appear via their sum or difference | merge them into one dimension | — |
| n up to 1e9, k small, structure splits in half | **doubling DP**: build n from n/2 then increment, O(k^2 log n) | binary expansion of n |
| the state is an unordered multiset / sorted tuple | canonicalise by sorting; state count is about p(n) ~ e^(c sqrt n) | fine to n ~ 60-100 with a hash map |
| count configurations up to rotation/reflection | Burnside — `combinatorics/burnside.cpp` | — |
| boolean feasibility DP with a large inner dimension | **bitset the inner dimension** | any inner loop that is "OR of a shifted array" is bitsettable |
| "you may reorder the items", pairwise order decides everything | **exchange argument**: derive the comparator from the n=2 case, sort, then a plain prefix DP | — |
| count **distinct** subsequences / strings | last-occurrence subtraction: `dp[i] = 2 dp[i-1] - dp[last[c]-1]` | kills duplicates without a set |
| "exactly k" is hard, "at least k" is easy | **binomial inversion**: `exactly(k) = sum_{j>=k} (-1)^(j-k) C(j,k) atleast(j)` | — |
| insert in sorted order, cost depends on adjacency | **connected-component DP**: state (i, #components, cost, endpoints used); transitions = new / append / merge | components are all the future cares about |
| "partition into groups, cost = sum of (max - min)" | **open/close DP**: state (i, #open groups); each step adds `j*(a[i+1]-a[i])` | groups are open intervals over sorted values |
| huge space with only a few special cells | index by the sorted special items: `dp[i] = total - sum(paths first blocked at j)` | state space becomes O(#special^2) |
| "apply the transition 1e18 times" on a functional graph | binary lifting as DP: `up[k][v] = up[k-1][up[k-1][v]]` | matrix power's cheap cousin when the transition is a function |

## TRANSFORMATIONS

| DP shape | is really | payoff |
|---|---|---|
| bitmask DP with weighted transitions | shortest path on the subset lattice | Dijkstra / 0-1 BFS instead of a fixed layer order |
| a "count of pieces" dimension you cannot afford | Lagrangian relaxation — a penalty lambda **replaces** the dimension | drops a factor of k |
| slope trick vs aliens trick | the same object: slope trick keeps all of convex f(k); aliens evaluates ONE tangent | choose by whether you need all k or one |
| slope trick (`DP/slope_trick.cpp`) | min-cost-flow simulation with a regret heap | three vocabularies for one convexity fact |
| (max,+) convolution of two convex arrays | Minkowski sum of epigraphs = merging difference sequences | O(n^2) to O(n) |
| D&C optimisation | SMAWK on a monotone matrix | SMAWK is strictly stronger and linear |
| Knuth optimisation | optimal-BST / Hu-Tucker structure | recognisable from "merge two adjacent parts, cost = size" |
| CHT | lower envelope of lines = min-plus convolution against a linear array = convex hull of the dual points | lets you reuse a hull routine |
| matrix power of a linear recurrence | polynomial multiplication mod the characteristic polynomial (Kitamasa) | O(d^3 log n) to O(d^2 log n) |
| DP values obeying an unknown linear recurrence | Berlekamp-Massey on a brute-forced prefix — `math/linear_recurrence.cpp` | you never need to derive the recurrence |
| counting DP with a product structure | a generating-function product; "sets of connected things" = exp of an EGF | closed forms, FFT, Newton |
| self-referential convolution DP | an implicit power-series equation | relaxed convolution / Newton |
| tree DP + point updates | a (max,+) matrix product along HLD chains | static DP becomes O(log^2 n) per update |
| expected-value DP with cycles | a linear system (Markov chain) | DAG -> DP, general -> Gauss, tree -> linear solve |
| a self-loop in a probability DP | a geometric series | divide by (1 - p_self) |
| digit DP | DP on the automaton recognising "<= N"; `tight` IS the automaton state | extra conditions = product automaton |
| SOS / zeta over subsets | multidimensional prefix sums over {0,1}^n; Mobius is the difference transform | same shape as the divisor-lattice transform |
| subset-sum convolution | ordinary OR-convolution once a popcount RANK dimension is added | rank is what enforces disjointness |
| tree knapsack with a numeric capacity | 1D knapsack over the Euler tour with skip-jumps (`DP/tree_knapsack.cpp`) | removes child merging entirely |
| "delete one element, recompute" | D&C over the index range carrying the outside-DP | n recomputations become log n |
| counting up to symmetry | Burnside — `combinatorics/burnside.cpp` | — |
| LIS via patience / BIT | DP over a segment tree indexed by value | generalises to weighted LIS |
| alternating-player score game | longest path with alternating negation; storing the DIFFERENCE collapses both players | one array instead of two |

## TRAPS

- **Min-plus identity** is 0 on the diagonal and +INF off it, not all-zero. INF must be <= LLONG_MAX/4.
- **Unreachable states**: for max-DP, initialising to 0 instead of -INF quietly legalises impossible states. For counting, the empty state is 1, not 0.
- **Mod kills comparison.** Never max/min values already reduced mod p. "Optimise then count optima" needs two arrays — value unmodded, count modded.
- **Negative residues** from last-occurrence subtraction and inclusion-exclusion: add p before storing.
- **Aliens tie-breaking**: with integer lambda several values tie on a flat segment, so the piece count is not k. Track the min (or max) count among optima and return `v(lambda) + lambda*k`. If the objective is not convex you get only a bound, silently.
- **D&C-opt vs 1D/1D**: D&C needs opt-monotonicity AND that `w(j,i)` does not depend on `dp[j]`. If the cost depends on the DP value, D&C is wrong.
- **Knuth needs two conditions**: the quadrangle inequality AND cost monotonicity on nested intervals. Diff `opt[i][j]` against brute force at n <= 10.
- **CHT numerics**: compare intersections by cross-multiplication in `__int128`, never division. Dedupe equal slopes by intercept.
- **Monotone deque discipline**: pop the front by INDEX (window bound) and the back by VALUE. Bounded knapsack needs one deque per residue class mod w.
- **Knapsack loop direction**: descending capacity = each item once; ascending = unlimited. The wrong one compiles, runs, and lies.
- **Digit DP memo scope**: share only for `tight == false`. Do not reuse the table between the L-1 and R passes without clearing.
- **Memo across test cases**: use a version stamp, not a memset of a 2^20 table per case.
- **Submask enumeration** `for (s = m; s; s = (s-1) & m)` never emits `s = 0`.
- **SOS loop order**: bits outer, masks inner. Swapping computes something else and still runs.
- **Rerooting inversion**: "remove one child" needs an invertible merge. For max, keep prefix/suffix aggregates or the top two.
- **Tree knapsack caps**: inner loops to k instead of `min(k, size)` turn O(nk) into O(nk^2) with identical output.
- **Expectation direction**: define E[state] as expected ADDITIONAL cost and never mix it with "from the start" in one recurrence.
- **Games with cycles**: plain memoised win/lose DFS loops or calls an unresolved state a Loss. Use out-degree counting BFS from terminals; unlabelled means Draw.
- **Matrix power edges**: n = 0 is the identity, and most transition matrices only hold for n >= base. If coefficients depend on i, matrix power is the wrong tool.
- **Interval DP double count**: fix once whether the split point k belongs left or right.
- **Recursion depth**: memoised DP over 1e5+ states on a chain stack-overflows.
- **Plug DP canonicalisation**: reduce connectivity labels to minimum representation before hashing, or one physical state occupies many keys.
- **Slope trick tags**: both heaps carry lazy add/shift tags; forgetting to apply one when moving an element across is THE bug. Adding a concave function destroys the invariant.
