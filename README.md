# 3ERSAN-EL-GANNA — reference

**Start at [`subproblems/README.md`](subproblems/README.md)** — that is the
router. Read a statement, find the row, open the file it names.

`template/template.cpp` is the boilerplate everything else pastes into.
Geometry and number theory are the other team's printed PDFs, not in here;
`subproblems/geometry.md` and `subproblems/number_theory.md` route into them.

---

## PRINT ORDER

If page count is limited, print in this order — it front-loads what gets
opened most.

| tier | what | pages |
|---|---|---|
| 1 | `subproblems/` (all 10) + `template/` | ~13 |
| 2 | `graph/shortest path/`, `graph/SCC/`, `DS/segment tree/`, `DS/BIT/`, `DS/LCA/` | ~30 |
| 3 | `strings/`, `hashing/`, `math/` | ~40 |
| 4 | `DP/`, `graph/flows/`, `graph/matching/`, the rest of `DS/` | ~40 |
| 5 | `game/`, `combinatorics/`, `stress/`, `snippet/` | ~10 |

---

## LAYOUT

### `subproblems/` — the index, no code
`README.md` (router, transformations, n→complexity) · `graph.md` · `ds.md`
· `strings.md` · `dp.md` · `math.md` · `combinatorics.md` · `game.md`
· `number_theory.md` · `geometry.md`

### `graph/`
- **`shortest path/`** — `dijkstra` · `01 bfs` · `bellman ford` ·
  `floyd warshall` · `dag paths` (toposort) · `k_shortest_paths`
- **`SCC/`** — `tarjan - bridges` (SCC + bridges + articulation + bridge tree)
  · `block_cut_tree` · `two_sat` · `eulerian_path` · `Tarjan SCC`
- **`MST/`** — `kruskal with dsu` · `prim mst` · `second_best_mst`
- **`flows/`** — `Dinics` · `altdinic` · `MCMF` ×2 · `edmonds karp`
- **`matching/`** — `kuhn` (+ König) · `hopcroft_karp` · `hungarian`
- **`dsu/`** — `dsu parity` · `dsu_rollback` · `dsu_marking`
- `kirchhoff` · `graph_coloring` · `inverse_graph` · `path_intersection`

### `DS/`
- **`segment tree/`** — `lazy` · `segtree_iterative` · `segtree_2d` ·
  `implicit` · `persistent/` · `merge/`
- **`BIT/`** — `pointxrange` · `rangexrange` · `fenwick_2d` ·
  `fenwick_2d_range` · `fenwick_2d_offline` · `fenwick_kth`
- **`sparse/`** — `Sparse Table` · `disjoint_sparse_table` · `sparse_table_2d`
- **`LCA/`** — `binary lifting` · `virtual_tree`
- **`HLD/`** · **`centroid/`** · **`Trie/`** · **`SQRT/`** (`mo`, `mo_on_tree`)
- `bitset` · `wavelet_tree` · `monotonic_stack` · `monotonic_deque` ·
  `prefix_sum_2d` · `2d partial sum` · `bst/treap_implicit`

### `DP/`
`rerooting` · `incexc` · `lis` · `sos` · `subset_sum_bitset` ·
`bitmask_tsp` · `broken_profile_dp` · `digit dp` · `digit_dp_automaton` ·
`sub-problems xorbasis` · **`optimization/`** (`CHT`, `li_chao`, `D&C`,
`knuth`) · **`matrix/`** (`matrix expo`, `minlenK`)

### `strings/` and `hashing/`
`Z kmp` · `aho_corasick` · `suffix_automaton` · `cleansuf` (suffix array) ·
`separate manacher` · `Z manacher` · `suffix array` · `subsuf`
— `hashing/`: `hash` · `hash_seg` · `hash_tree` · `hash_ms` · `hash_2d`
(see `hashing/README.md`)

### `math/`
`modular` · `extgcd` · `crt` · `phi` · `bsgs` · `gauss` · `fft_ntt` ·
`fwht` · `convolution_anymod` · `bitwise` · `randomization` ·
`compare_fractions`

### the rest
`combinatorics/` (`catalan`, `nCr_with_mod`, `nCr_without_mod`) ·
`game/grundy` · `number theory/sieve` · `stress/gen` ·
`snippet/` (`ordered_set`, `compress`)

---

## CONVENTIONS

- Most files are **1-indexed**; anything else says so in its header.
- `l + r >> 1` is deliberate and correct — `+` binds tighter than `>>`.
- A blank constant (`const int mod = ;`) is a **placeholder you fill per
  problem**, not a bug.
- Files split by `////////` hold **two variants** — paste one half.
- A `####` box marks a mistake that costs real debugging time. Read those.
- Some files need another's helpers; the header says which.

## TESTING

`stress/gen.cpp` has the generators — seven tree shapes (not just random:
a uniform tree is depth O(log n) and will not find your stack overflow),
connected/DAG/bipartite graphs, and the compare-loop driver.
