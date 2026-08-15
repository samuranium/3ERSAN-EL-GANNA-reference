# SUBPROBLEMS — what to reach for when you see X

Read this when you have a statement and no plan yet. Left column is what the
**statement** says. Right column is where to go.

Per-category files: `graph.md` `ds.md` `strings.md` `dp.md` `math.md`
`combinatorics.md` `number_theory.md` `geometry.md` `game.md`

Geometry and number theory live in the other team's printed PDFs, not in this
repo. Their entries route to a PDF section.

---

## 1. FIRST QUESTION — what is the object?

| the problem is about | go to |
|---|---|
| cities/roads, nodes/edges, dependencies, "reachable" | `graph.md` |
| a tree, parent/child, subtrees, paths in a tree | `graph.md` § TREES |
| an array with queries on ranges | `ds.md` |
| a grid | `ds.md` § 2D, `graph.md` § GRID |
| a string, substrings, patterns | `strings.md` |
| "count the ways", "how many" | `combinatorics.md`, `dp.md` |
| "minimum/maximum cost to ..." with choices | `dp.md`, `graph.md` § FLOW |
| divisibility, primes, modulo | `number_theory.md` |
| points, lines, polygons, areas | `geometry.md` |
| two players alternate | `game.md` |
| a matrix, linear equations, polynomials | `math.md` |

---

## 2. TRANSFORMATIONS — X is secretly Y

The most valuable table here. These are the reductions that are hard to see
under pressure.

| you have | it is really | where |
|---|---|---|
| each item has exactly **two** options, pairs conflict | 2-SAT | `graph/SCC/two_sat.cpp` |
| each item has **three or more** options | flow / matching, **not** 2-SAT | `graph.md` § FLOW |
| "use every **edge** once" | Eulerian path | `graph/SCC/eulerian_path.cpp` |
| "use every **vertex** once" | Hamiltonian → NP-hard, bitmask DP if n ≤ 20 | `dp.md` § BITMASK |
| inequalities `x_j − x_i ≤ c` | shortest path | `graph/shortest path/bellman ford.cpp` [5] |
| "count the special moves used" | 0-1 BFS | `graph/shortest path/01 bfs.cpp` |
| **longest** path | must be a DAG first | `graph/shortest path/dag paths.cpp` |
| pair up two disjoint groups | bipartite matching | `graph/matching/kuhn.cpp` |
| "minimum vertices to cover all edges" | = max matching (König) | `graph/matching/kuhn.cpp` |
| "maximum set with no edge inside" | n − max matching | `graph/matching/kuhn.cpp` |
| partition into two groups, penalties | min cut | `graph/flows/Dinics.cpp` |
| "each X must be used **at least** k times" | flow with lower bounds | `graph/flows/lower_bounds.cpp` |
| "minimise the cost **per step** around a loop" | min mean cycle | `graph/min_mean_cycle.cpp` |
| complete graph, weight given by a **formula** | Borůvka (xor → binary trie) | `graph/MST/boruvka.cpp` |
| "nobody would rather swap" | stable matching | `graph/matching/gale_shapley.cpp` |
| "connect these k special nodes" | Steiner tree DP | `graph/steiner_tree.cpp` |
| edges to a whole **range** of nodes | segment tree graph | `graph/segment_tree_graph.cpp` |
| "min possible **max** edge on a route" | Kruskal reconstruction tree | `graph/MST/kruskal_tree.cpp` |
| a convex piecewise-linear cost you keep adding to | slope trick | `DP/slope_trick.cpp` |
| the game graph has **cycles** or draws | retrograde BFS | `game/retrograde.cpp` |
| the answer is a polynomial / linear recurrence in n | interpolation / Berlekamp–Massey | `math/interpolation.cpp`, `math/linear_recurrence.cpp` |
| "every X appears an even number of times" | GF(2) vector, xor basis | `DP/sub-problems xorbasis.cpp` |
| "xor of a subset" | xor basis | `DP/sub-problems xorbasis.cpp` |
| "max xor of a **pair**" | binary trie | `DS/Trie/binary_trie.cpp` |
| combine every pair, index by the **sum** | convolution (FFT/NTT) | `math/fft_ntt.cpp` |
| combine every pair, index by **xor/and/or** | FWHT, **not** FFT | `math/fwht.cpp` |
| "for each mask, aggregate over submasks" | SOS DP | `DP/sos.cpp`, `dp.md` |
| a range op where each cell is touched **once** | DSU as next-pointer | `graph/dsu/dsu_marking.cpp` [A] |
| queries offline, edges appear/disappear | segment tree on time + rollback DSU | `graph/dsu/dsu_rollback.cpp` [1] |
| tree path queries **with updates** | HLD | `DS/HLD/` |
| tree path queries, **no** updates | LCA + prefix sums | `DS/LCA/binary lifting.cpp` |
| "answer for every root" | rerooting DP | `DP/rerooting.cpp` |
| k marked nodes, Σk bounded, n huge | virtual tree | `DS/LCA/virtual_tree.cpp` |
| "repeat this operation k times", k ≤ 1e18 | binary lifting / matrix power | `DS/LCA/binary lifting.cpp` [T7], `DP/matrix/` |
| a boolean inner loop over n | bitset, ÷64 | `DS/bitset.cpp` |
| "is sum s reachable" (not: how many ways) | bitset subset-sum | `DS/bitset.cpp` [1] |
| independent piles, a move touches one | Sprague–Grundy, xor | `game/grundy.cpp` |
| counting spanning trees | Kirchhoff determinant | `graph/kirchhoff.cpp` |
| a DP whose transition is a line `m·x + c` | CHT / Li Chao | `DP/optimization/` |
| grid distances with only ↑↓←→ | BFS; if diagonal too → Chebyshev rotation | `geometry.md` |
| "at most k of something" in a DP | add k as a state dimension first, optimise later | `dp.md` |

---

## 3. SIZE TELLS YOU THE ALGORITHM

| n up to | intended complexity | typical tool |
|---|---|---|
| 10–12 | n! | permutations, brute force |
| 20–24 | 2ⁿ, 3ⁿ | bitmask DP, submask DP, meet in the middle |
| 40 | 2^(n/2) | meet in the middle |
| 100–500 | n³ | Floyd, matrix, Gauss, interval DP |
| 2 000–5 000 | n² | DP over pairs, n² graph algorithms |
| 10⁵–10⁶ | n log n | sort, segment tree, binary search |
| 10⁷–10⁸ | n | two pointers, prefix sums, sieve |
| 10¹⁸ | log n | binary lifting, matrix power, math |

If n ≤ 20 the answer is almost always a bitmask. If n is 10⁵ and the natural
solution is O(n²), the intended one is a data structure or a sort.

---

## 4. WHEN STUCK — reductions to try in order

1. **Sort it.** Many "for each pair" problems collapse once sorted.
2. **Reverse it.** Process queries backwards; reverse the edges; think from the
   answer to the input.
3. **Binary search the answer.** If checking a guess is easier than finding it.
4. **Split at the middle.** Meet in the middle, D&C, centroid.
5. **Contribution.** Instead of "value of each object", ask "how many objects
   does each element contribute to".
6. **Complement.** Count the bad ones and subtract.
7. **Add a dimension.** Carry the awkward constraint as DP state, then look for
   a way to drop it.
8. **Make it a graph.** States are vertices, moves are edges — even when the
   statement never mentions a graph.
