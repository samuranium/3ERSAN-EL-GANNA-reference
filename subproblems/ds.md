# DATA STRUCTURES — subproblems

## PICK BY THE QUERY/UPDATE PAIR

| update | query | tool | file |
|---|---|---|---|
| none | range sum | prefix sums | `DS/prefix_sum_2d.cpp` (1D is trivial) |
| none | range min/max/gcd | sparse table, O(1) | `DS/sparse/Sparse Table.cpp` |
| none | range sum/product, no inverse | disjoint sparse table, O(1) | `DS/sparse/disjoint_sparse_table.cpp` |
| point | range sum | Fenwick | `DS/BIT/pointxrange.cpp` |
| point | range any associative | iterative segment tree | `DS/segment tree/segtree_iterative.cpp` |
| range | point | difference array, or Fenwick | `DS/BIT/rangexrange.cpp` |
| range | range | lazy segment tree | `DS/segment tree/lazy.cpp` |
| range | range, offline only | difference array + one sweep | `DS/2d partial sum.cpp` |
| point | range, but 2D | 2D Fenwick | `DS/BIT/fenwick_2d.cpp` |
| none | rectangle min/max | 2D sparse table | `DS/sparse/sparse_table_2d.cpp` |
| insert/erase **at a position** | range | implicit treap | `DS/bst/treap_implicit.cpp` |
| none, but versions persist | range | persistent segment tree | `DS/segment tree/persistent/` |
| coordinates up to 1e9 | range | implicit / dynamic segment tree | `DS/segment tree/implicit.cpp` |

## BY WHAT IS ASKED

| you see | tool | file |
|---|---|---|
| k-th smallest in a multiset | Fenwick order statistics | `DS/BIT/fenwick_kth.cpp` |
| k-th set bit / k-th free slot | same | `fenwick_kth.cpp` |
| rank of a value | `pre(v)` | `fenwick_kth.cpp` |
| number of inversions | Fenwick sweep | `fenwick_kth.cpp` |
| k-th smallest in a **range** | wavelet tree, or persistent segtree | `DS/wavelet_tree.cpp` |
| ... and you also need rank / quantile | wavelet tree | `DS/wavelet_tree.cpp` |
| ... across persistent versions | persistent segment tree | `DS/segment tree/persistent/persistent.cpp` |
| count values ≤ x in a range | wavelet tree, or merge sort tree | `DS/wavelet_tree.cpp` |
| first index ≥ L with value ≥ x | segment tree descend | `DS/segment tree/lazy.cpp` [1] |
| nearest smaller / greater element | monotonic stack | `DS/monotonic_stack.cpp` |
| largest rectangle in a histogram | monotonic stack | `monotonic_stack.cpp` |
| Σ over subarrays of their minimum | monotonic stack, contribution | `monotonic_stack.cpp` |
| sliding window min/max | monotonic deque | `DS/monotonic_deque.cpp` |
| longest subarray with max−min ≤ x | two deques + two pointers | `monotonic_deque.cpp` |
| max subarray sum, with updates | segment tree, custom node | `lazy.cpp` [2] |
| range gcd **with** range add | segment tree over the difference array | `lazy.cpp` [3] |
| ordered set with `k-th` and `rank` | pb_ds, or Fenwick | `snippet/ordered_set.cpp` |
| range queries, no updates, offline | Mo's algorithm | `DS/SQRT/mo.cpp` |
| distinct values in a range | Mo's, or persistent segment tree | `DS/segment tree/persistent/persistent count distinct.cpp` |
| assign each element once, over ranges | DSU next-pointer | `graph/dsu/dsu_marking.cpp` |
| reverse a subarray / cut and paste | implicit treap | `DS/bst/treap_implicit.cpp` |

## WHEN THE INNER LOOP IS BOOLEAN → BITSET

`DS/bitset.cpp`. Everything here is ÷64.

| you see | do this |
|---|---|
| "is sum s reachable" | `dp \|= dp << w` |
| reachability closure | `if (r[i][k]) r[i] \|= r[k]` |
| common neighbours / triangles | `(adj[u] & adj[v]).count()` |
| string matching, count in a range | AND of shifted character bitsets |
| GF(2) elimination | rows as bitsets | `math/gauss.cpp` |
| iterate only the set bits | `_Find_first` / `_Find_next` |

**Not** a bitset when you need counts or sums rather than feasibility.

## THE DECISION THAT IS USUALLY WRONG

- **Sparse table with a non-idempotent op.** The two blocks overlap, so `sum`
  double-counts. Use a disjoint sparse table.
- **Fenwick with both range-update and range-query on the same struct.** Those
  are different modes; see the note in `DS/BIT/pointxrange.cpp`.
- **Segment tree where a Fenwick would do.** Half the code, four times faster.
- **Treap where the positions never change.** Use a segment tree.
- **Mo's when the update is online.** Mo's is offline only.

## COMPLEXITY REMINDERS

| structure | build | query | update |
|---|---|---|---|
| prefix sums | O(n) | O(1) | rebuild |
| sparse table | O(n log n) | O(1) | rebuild |
| Fenwick | O(n) | O(log n) | O(log n) |
| segment tree | O(n) | O(log n) | O(log n) |
| lazy segment tree | O(n) | O(log n) | O(log n) range |
| treap | O(n) | O(log n) | O(log n) anywhere |
| Mo's | — | O((n+q)√n) total | offline |
| bitset op | — | O(n/64) | O(n/64) |
