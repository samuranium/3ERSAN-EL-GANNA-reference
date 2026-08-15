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
| range | range, but 2D and sums only | four 2D Fenwicks | `DS/BIT/fenwick_2d_range.cpp` |
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
| the same, but on TREE PATHS | Mo's on the Euler tour | `DS/SQRT/mo_on_tree.cpp` |
| values up to 1e9, need array slots | coordinate compression | `snippet/compress.cpp` |
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


---

# RESEARCH ADDITIONS

## SQRT / BLOCK FAMILY

| you see | reach for | the tell |
|---|---|---|
| update is trivial, the query needs a **built** form (sorted, hull, table) | **sqrt on operations**: rebuild the static structure every B ops, keep <= B pending in a buffer, answer = static + brute over the buffer | B = sqrt(q). Converts "no dynamic version exists" into "static + small patch" |
| the aggregate has no mergeable form at all (mode, range majority) | block decomposition + **block-pair answer table** | the answer is that, or an element from the two partial blocks |
| range **mode**, static | block-pair mode table + per-value sorted occurrence list | mode never decreases when you extend; verify candidates by binary search |
| Mo's, but `remove()` is hard or impossible | **rollback Mo's** (add-only + undo) | fix L's block, R monotone, undo the left extension per query. Never calls remove |
| range queries interleaved with **point assignments**, offline | **Mo's with updates (3D Mo)**, block = n^(2/3), O(n^(5/3)) | third key = number of updates before the query |
| Mo's is right but TLEs on constant factor | **Hilbert curve order**, or block = n/sqrt(q) with alternating R direction | plain sqrt(n) blocks are wrong when q << n |
| static range aggregate, **non-idempotent**, want O(1) | **sqrt tree** — O(n log log n) build, O(1) query, O(sqrt n) point update | sparse table cannot; disjoint sparse table is O(1) but static |
| insert/erase at arbitrary position + range aggregate, n <= 2e5 | **sqrt buckets / unrolled list**, rebuild every sqrt n inserts | much shorter than a treap, better cache behaviour |
| "count <= x" with point updates, values bounded | buckets over the **value axis** (sqrt C blocks), O(1) update / O(sqrt C) query | flips the asymmetry when updates >> queries |

## SEGMENT TREE VARIANTS

| you see | reach for | the tell |
|---|---|---|
| range `a[i] = min(a[i], x)` **plus range sum** | **segment tree beats** — node keeps (max, 2nd max, count of max, sum) | chmin does not compose as a lazy tag; amortised O(n log n), O(n log^2 n) with range add |
| `min over [l,r] of (a_i + b_i*t)`, t **non-decreasing across queries** | **kinetic segment tree** — node stores winner + melt time | one out-of-order t destroys the amortisation |
| many per-node value-indexed structures merged up a tree | **segment tree merging** | O(n log n) total because each recursion destroys a node; beats small-to-large's extra log |
| "count of elements with **value** in [a,b]" | segment tree indexed by **value**, not position | the tell: the query range is on the value axis |
| coordinates to 1e9 and **forced online** (queries xor'd with the last answer) | dynamic / sparse segment tree | cannot compress offline — future coordinates are unknown |
| "as of prefix i" / "as of version t" | persistent segment tree over prefixes; subtract version l-1 from r | gives k-th, count <= x, distinct-in-range, all **online** |
| "how many in [l,r] are <= x", static, simplest | merge sort tree O(log^2 n); + **fractional cascading** for O(log n) | binary search once at the root, push the position down in O(1) |
| track "max value this cell ever held" under range add | **historic segment tree**: tag = (current, historic max of the tag prefix) | tags do not commute; `hist = max(hist, own_lazy + child_hist)` |
| point update, query = a linear-recurrence / automaton DP over a range | **segment tree of matrices** | node = the transfer operator for its range; merge = matrix product, order matters |
| range min **and how many achieve it** | node = (min, count) | needed before any beats-style trick |
| **range assignment** dominates the operation mix | **Chtholly tree / ODT** (map of equal-value intervals) | each assign destroys at least as many intervals as it creates. Adversarial input with no assign is O(n) per op |
| offline deletions from an insert-only structure | segment tree over the **time axis** + rollback | each element occupies O(log q) nodes covering its live interval |

## OFFLINE <-> ONLINE CONVERSION

| you see | reach for | the tell |
|---|---|---|
| structure supports insert but not delete, all ops known | **offline deletion** (segment tree on time + rollback) | needs rollback and **no** path compression / no amortisation |
| "for each query, the earliest moment it becomes satisfiable" | **parallel binary search** | all queries share one time axis; process the common midpoint once |
| three simultaneous inequalities / dynamic 2D counting | **CDQ divide and conquer** | sort dim 1, D&C on dim 2, BIT on dim 3. The D&C is over TIME when the problem is insert-then-query |
| only a static structure exists but you need insertions | **logarithmic method / binary counter**: keep log n static structures of size 2^k, merge on carry | amortised O(log n) rebuilds per insert; query = fold over log n structures |
| the problem is **forced online** | persistence / dynamic segtree / LCT are the only options | Mo's, CDQ, parallel binary search and offline sweeps are all disqualified — check this FIRST |
| "count pairs satisfying a 2D condition" | offline sweep by one coordinate + BIT on the other | the sort order IS the second dimension |
| "number of distinct in [l,r]", offline | sort queries by r; BIT +1 at each value's latest occurrence, -1 at its previous | removes the need for Mo's entirely, O((n+q) log n) |

## TREES

| you see | reach for | the tell |
|---|---|---|
| static tree, path updates + path queries | HLD, O(log^2 n) (O(log n) if the op is invertible: root-prefix subtraction) | tree edges never change |
| static tree, **subtree** update / query | Euler tour + BIT | subtree = one contiguous range. Do not reach for HLD |
| path **sum** with point updates | Euler tour with two entries per node (+x at tin, -x at tout+1) + BIT | root-to-node prefix; invertible ops only |
| tree **topology changes** (link/cut) + path aggregates | link-cut tree | HLD would need full re-decomposition |
| topology changes + **subtree** aggregates or connectivity only | Euler tour tree | LCT is path-oriented; subtree on LCT needs virtual-subtree augmentation |
| topology changes + BOTH path and subtree lazy | top tree / rake-compress | the only clean option, huge constant, last resort |
| many LCA queries, offline, memory-bound | Tarjan offline LCA with DSU, O((n+q) alpha) | but binary lifting also gives k-th ancestor |

## HEAPS, MONOTONE, MISC

| you see | reach for | the tell |
|---|---|---|
| sliding window fold with an **associative non-invertible** op (min, gcd, OR, matrix product) | **SWAG** — queue of two stacks with prefix folds, amortised O(1) | monotonic deque only does min/max; SWAG does any monoid |
| `dp[i] = dp[i-1] + abs(x - a_i)`-shaped convex piecewise-linear cost | **slope trick** — heap of slope-change points | cost is convex, piecewise linear, continuous |
| sliding-window median / k-th | two heaps with **lazy deletion**, or a BIT over compressed values | mark evicted, pop only when it surfaces |
| "delete an arbitrary element from a priority queue" | a second "deleted" heap, pop while the tops match | avoids multiset's constant factor |
| max XOR over a **subarray** | **persistent** binary trie over prefix XORs | version subtraction gives the range restriction |
| connectivity queries that must be undone | DSU with rollback: union by size, **no** path compression | O(log n), not alpha(n) — the price of undo |
| D&C where the split point is the range min/max | **Cartesian tree** (build in O(n) with a monotonic stack) | makes "sum over subarrays of f(min)" a subtree-contribution problem; also O(1) RMQ via LCA |
| "count subarrays whose gcd / AND / OR equals k" | stack of (value, count) per right endpoint | only O(log C) distinct values of gcd/AND/OR over suffixes ending at r |
| maintain intervals with merge-on-insert (booking, free space) | map of start -> end, or Chtholly if values are assigned | each merge deletes at least one interval |

## TRANSFORMATIONS

| pattern | actually is | mechanism |
|---|---|---|
| "count pairs i<j with a_i > a_j" | BIT over compressed values, swept left to right | each element queries the suffix already inserted |
| "how many intervals contain point x" | difference array + prefix sum, events sorted | the 2D-looking query collapses once you sort by one axis |
| "k-th smallest in [l,r]" | persistent segtree on prefixes, walk down on (right - left) | version subtraction gives the range's multiset free |
| "max edge on path u,v" / "reachable using weights <= w" | Kruskal reconstruction tree | threshold becomes an ancestor, component becomes a subtree |
| "assign v to all of [l,r]; also range sum" | ODT / interval map | assignment is destructive — interval count only shrinks |
| "add x to every node within distance d of v" | centroid decomposition, or Euler tour + depth-indexed BIT | distance factors through O(log n) ancestor centroids |
| "query on a root-to-node path", invertible op | prefix(u) o prefix(v) o prefix(lca)^-2 | no HLD needed when the op has inverses |
| "for each query, the first time a condition holds" | parallel binary search | every query searches the same axis |
| "count triples dominated in 3 dimensions" | CDQ: sort dim1, D&C dim2, BIT dim3 | update time is one of the dimensions |
| "sliding window of min / gcd / OR / matrix product" | SWAG | non-invertible means no prefix subtraction |
| "sum over all subarrays of f(min)" | monotonic stack / Cartesian tree contribution | each element is the min of a rectangle of (l,r) pairs |
| "maintain a multiset, how many <= x" | BIT over compressed values, **not** multiset | multiset has no rank; std::distance is O(n) |
| "repeatedly assign to unassigned positions" | DSU next-pointer, or process queries in reverse | each cell is written once |
| "merge the sets of two children, answer per node" | small-to-large / DSU on tree / segment tree merging | choose by cost per element: map insert -> small-to-large; array counter -> sack; value-indexed aggregate -> segtree merge |
| "historic maximum under range add" | segment tree with a (current, historic) tag pair | historic is a PREFIX max over the tag sequence |
| "insert-only structure but the problem deletes" | offline deletion, or the logarithmic method | both trade a log for the missing operation |

## TRAPS

- **Identity is not identity.** 0 as the identity for min; INT_MAX combined with add-lazy overflows. Use LLONG_MAX/4 and check `merge(x, id) == x` for **every** field.
- **Lazy composition order for assign + add.** Applying add to a node with a pending assign must fold into the assign; applying assign must **erase** the pending add. Two independent tags is the classic silent WA.
- **Lazy scaling by length** applies to sum, not to min/max. A node holding both needs two apply rules.
- **Iterative segment tree + non-commutative merge**: accumulate left and right separately and combine at the end.
- **Historic tags do not commute.** You need the tag's timestamp; summing lazies loses the intermediate peaks.
- **Beats degradation**: chmin + chmax + add together is O(n log^2 n). Handle `max1 == max2` explicitly or you loop forever.
- **Rollback forbids path compression.** Union by size only; you drop from alpha(n) to Theta(log n).
- **Amortised structures break under rewind.** Splay, path-compressed DSU, Chtholly and SWAG all lose their bounds when an adversary rewinds or the assumed op mix is absent.
- **Persistent segtree memory**: about (n+q)*log n nodes. n=q=2e5 gives 4-8e6 nodes — preallocate flat arrays, never `new` per node.
- **Persistence + lazy**: push must CREATE nodes, not mutate. Persistent + amortised (beats, splay) is unsound.
- **Segment tree merging is destructive.** You cannot merge then query the consumed tree, and you cannot merge persistent trees.
- **4n vs 2n sizing.** Recursive needs 4n; bottom-up needs 2n but leaf order only matches the array when n is a power of two.
- **Sparse table**: precompute the log table; `log2()` per query is a real TLE.
- **Fenwick is 1-indexed.** Index 0 makes `i += i & -i` loop forever. Prefix-max Fenwick is valid only if values move monotonically upward.
- **pbds ordered_set faked into a multiset** with `less_equal` breaks find, erase and the bound functions. Insert `pair<value, counter>` instead.
- **`multiset::erase(value)` erases every copy.** Use `s.erase(s.find(v))`.
- **Coordinate compression must include query endpoints**, and you must fix `<` vs `<=` before compressing.
- **Mo's block size** is n/sqrt(q), not sqrt(n), when q << n. The answer must be a pure function of the window.
- **Kinetic segment tree requires monotone t.** One smaller t forces an O(n) rebuild.
- **Small-to-large done backwards** (iterating the larger) is O(n^2) and looks identical.
- **Treap RNG**: a fixed seed or `rand()` gives priority collisions and O(n) depth. Seed `mt19937` from the clock.
- **Sqrt block size as a compile-time constant** lets the compiler turn `/` and `%` into shifts — often AC vs TLE.
