# GRAPH — subproblems

## SHORTEST PATH — pick by the weights

| weights | tool | file |
|---|---|---|
| all equal | BFS | — |
| 0 or 1 (or 0 / k) | 0-1 BFS, deque | `graph/shortest path/01 bfs.cpp` |
| all ≥ 0 | Dijkstra | `graph/shortest path/dijkstra.cpp` |
| any negative | Bellman–Ford / SPFA | `graph/shortest path/bellman ford.cpp` |
| any, but the graph is a DAG | topological order | `graph/shortest path/dag paths.cpp` |
| all pairs, n ≤ 500 | Floyd–Warshall | `graph/shortest path/floyd warshall.cpp` |
| negative edges + all pairs | Johnson = Bellman–Ford then Dijkstra | `dijkstra.cpp` [R6] |

**Subproblems**

| you see | do this |
|---|---|
| shortest from **every** node **to** t | reverse all edges, one run from t — `dijkstra.cpp` [R1] |
| best path s→t **through** v | `ds[v] + dt[v]` (forward + reverse runs) |
| is edge (u,v,w) on **some** shortest path | `ds[u] + w + dt[v] == ds[t]` |
| count shortest paths | `cnt[v] += cnt[u]` on ties |
| shortest path with fewest edges | compare `(dist, hops)` lexicographically |
| nearest of several sources | push all sources at distance 0 |
| fuel / keys / k free edges | put it in the state: `d[node][state]` |
| weight on **nodes** not edges | split `u` into `u_in → u_out` |
| **minimise the maximum edge** | relax with `max(d[u], w)`; or MST path |
| maximise the minimum edge | `min(d[u], w)` + max-heap; or max spanning tree |
| **longest** path | acyclic? → `dag paths.cpp`. Cyclic? → condense SCCs first |
| k shortest routes (repeats allowed) | `k_shortest_paths.cpp` |
| k shortest **simple** paths | Yen / Eppstein — not in the repo |
| "can you loop and gain forever" | negative cycle — `bellman ford.cpp` |
| system of `x_j − x_i ≤ c` | difference constraints — `bellman ford.cpp` [5] |
| timetable / "edge usable at time t" | node = (place, time) |

## CONNECTIVITY

| you see | tool | file |
|---|---|---|
| one-way roads, mutually reachable groups | SCC | `graph/SCC/tarjan - bridges.cpp` |
| condense to a DAG, then DP | SCC + build_dag | same |
| removing which **edge** disconnects | bridges, bridge tree | same |
| removing which **vertex** disconnects | articulation, block-cut tree | `graph/SCC/block_cut_tree.cpp` |
| two vertex-disjoint paths between u,v | same block (Menger, k=2) | `block_cut_tree.cpp` [5] |
| path u→v avoiding w | w on the block-cut tree path | `block_cut_tree.cpp` [1] |
| each item true/false, pairwise clauses | 2-SAT | `graph/SCC/two_sat.cpp` |
| use every edge exactly once | Eulerian path | `graph/SCC/eulerian_path.cpp` |
| chain words end-to-start | Eulerian on 26 letters | `eulerian_path.cpp` [2] |
| offline, edges appear and vanish | segment tree on time + rollback DSU | `graph/dsu/dsu_rollback.cpp` |
| union with a parity / bipartite check | DSU with parity | `graph/dsu/dsu parity.cpp` |
| "assign the first free slot ≥ x" | DSU next-pointer | `graph/dsu/dsu_marking.cpp` |

## FLOW / MATCHING

| you see | tool | file |
|---|---|---|
| max flow, min cut | Dinic | `graph/flows/Dinics.cpp` |
| partition into two sides with penalties | min cut = project selection | `Dinics.cpp` |
| bipartite matching | Kuhn (default) | `graph/matching/kuhn.cpp` |
| ... and Kuhn actually TLEs | Hopcroft-Karp, O(E√V) | `graph/matching/hopcroft_karp.cpp` |
| minimum vertex cover | = max matching (König) | `kuhn.cpp` |
| maximum independent set (bipartite) | n − max matching | `kuhn.cpp` |
| minimum path cover of a DAG | n − matching on the split graph | `kuhn.cpp` |
| maximum antichain in a poset | Dilworth → path cover | `kuhn.cpp` |
| does a perfect matching exist | Hall: every S has \|N(S)\| ≥ \|S\| | — |
| matching with **costs**, n = m | Hungarian, O(n³) | `graph/matching/hungarian.cpp` |
| matching with costs + capacities | MCMF | `graph/flows/MCMF.cpp` |
| minimise the **maximum** assigned cost | binary search + Kuhn, **not** Hungarian | `graph/matching/kuhn.cpp` |
| each edge has a lower bound too | flow with lower bounds | — |
| vertex capacities | split the vertex | `Dinics.cpp` |
| matching in a **general** graph | Blossom — not in the repo | — |

## TREES

| you see | tool | file |
|---|---|---|
| LCA, distance, k-th ancestor | binary lifting | `DS/LCA/binary lifting.cpp` |
| is u an ancestor of v | tin/tout, O(1) | same |
| k-th node on the path u→v | jump() | same |
| max/min/sum edge on a path | second lifting table | same [T1] |
| path queries **with updates** | HLD | `DS/HLD/` |
| subtree queries | Euler tour → range query | tin/tout + segment tree |
| answer for **every** root | rerooting DP | `DP/rerooting.cpp` |
| the farthest node from each vertex | rerooting, best-two form | `DP/rerooting.cpp` B |
| diameter | two BFS, or max eccentricity | `DP/rerooting.cpp` |
| k marked nodes, Σk small, n huge | virtual tree | `DS/LCA/virtual_tree.cpp` |
| count paths with a property | centroid decomposition | `DS/centroid/centroid.cpp` |
| meeting point of three nodes | deepest of the three pairwise LCAs | `binary lifting.cpp` [T3] |
| LCA when re-rooted at r | deepest of lca(u,v), lca(u,r), lca(v,r) | `binary lifting.cpp` [T4] |
| number of spanning trees | Kirchhoff | `graph/kirchhoff.cpp` |
| second-best spanning tree | MST + max edge on path | `graph/MST/second_best_mst.cpp` |
| queries on subtrees, offline | small-to-large, or DSU on tree | — |
| distinct values on a path, offline | Mo's on the tree | `DS/SQRT/mo_on_tree.cpp` |
| tree isomorphism | canonical hash of the subtree | `hashing/hash_tree.cpp` |
| path as a string | path hash | `hashing/hash_tree.cpp` |
| do two tree paths overlap, and where | six candidate LCAs, filter | `graph/path_intersection.cpp` |

## GRID

| you see | do this |
|---|---|
| move ↑↓←→, all cost 1 | BFS, cell = vertex |
| some cells cost extra | 0-1 BFS or Dijkstra |
| turning costs, going straight is free | node = (cell, direction), 0-1 BFS |
| break at most k walls | node = (cell, walls used) |
| count regions / islands | DSU or flood fill |
| largest all-ones rectangle | histogram per row + monotonic stack — `DS/monotonic_stack.cpp` |
| submatrix sums | `DS/prefix_sum_2d.cpp` |
| many rectangle updates, read at the end | `DS/2d partial sum.cpp` |

## MISC

| you see | tool | file |
|---|---|---|
| chromatic number, n ≤ 20 | subset DP inclusion–exclusion | `graph/graph_coloring.cpp` |
| BFS / components of the COMPLEMENT graph | unvisited-set walk, O(n+m) | `graph/inverse_graph.cpp` |
| is it bipartite | BFS 2-colour, or DSU parity | `graph/dsu/dsu parity.cpp` |
| minimum spanning tree | Kruskal / Prim | `graph/MST/` |
| bottleneck path between two nodes | max edge on the MST path | `second_best_mst.cpp` |
| topological order, lexicographically smallest | Kahn with a priority queue | `dag paths.cpp` [2] |
| cycle detection, directed | Kahn: `ord.size() != n` | `dag paths.cpp` [1] |
| functional graph, "after k steps" | binary lifting | `binary lifting.cpp` [T7] |


---

# RESEARCH ADDITIONS

## TREES — more routing

| you see | reach for | the tell |
|---|---|---|
| per-subtree most frequent / distinct count, offline | **DSU on tree (sack)** — keep the heavy child's counters, re-add light subtrees | O(n log n) with plain arrays, no map log |
| merge sets/maps up the tree, answer per node | small-to-large, or **segment tree merging** | segtree merge is O(n log n) total AND supports k-th / range-count; maps cannot |
| "choose exactly k in each subtree" | tree knapsack, loop capped by **merged subtree size** | looks O(n^3), is O(n^2) — every pair is charged once, at its LCA |
| max independent set / min vertex cover / dominating set on a tree | 2-state (3-state for dominating) tree DP | NP-hard in general, linear here |
| "delete edges so each component satisfies P" | tree DP carrying the component statistic, cut bottom-up | deepest-valid-cut greedy is optimal for monotone P |
| sum of distances over all pairs / count pairs at distance >= d | **contribution per edge**: each edge splits into a*b pairs | turns a path sum into n independent O(1) counts |
| add x on many paths, read all values at the end | tree difference: +x at u and v, -x at lca and par(lca), one subtree sum | no HLD needed |
| minimise tree height by choosing the root | **center** = diameter midpoint | NOT the centroid |
| count labelled trees with given degrees | Prufer: (n-2)! / prod(d_i - 1)!; Cayley n^(n-2) | deg(v) = occurrences in Prufer + 1 |

## CONNECTIVITY — structure facts

| you see | the fact |
|---|---|
| "orient every road, keep everyone reachable" | **Robbins**: possible iff bridgeless. DFS tree edges down, back edges up |
| min edges to add for one SCC | max(#sources, #sinks) on the condensation — **0 if already one SCC** |
| min edges to add for 2-edge-connectivity | ceil(leaves / 2) on the bridge tree |
| who lies on EVERY path s to t | **dominator tree**; on a DAG it is idom(v) = LCA of all preds in topo order |
| remove one vertex/edge, count disconnected pairs | block-cut / bridge tree + subtree sizes, sum a*b |
| every edge in at most one cycle | **cactus** — blocks are bridges or single cycles; path counts multiply by 2 per cycle |
| n vertices and n edges | unicyclic: find the cycle, hang trees off it. Directed analogue = functional graph |
| count reachable pairs, n <= 5e4, DAG | bitset closure in reverse topo order, O(nm/64) |
| "drawn on a map, no crossings" | planar: m <= 3n-6, some vertex has degree <= 5, 4-colourable |

## BUILDING THE GRAPH YOU WERE NOT GIVEN

| you see | do this |
|---|---|
| edge from u to **every node in [l,r]** | **segment tree graph**: out-tree (parent to child, w=0) + in-tree (child to parent, w=0); 2n nodes, O(n log n) edges, then plain Dijkstra |
| "all nodes of this colour/row are mutually connected at cost w" | one virtual node per group, in-edge 0 / out-edge w — collapses an implicit clique to a star |
| path labels must match a pattern | product graph with the automaton: Dijkstra on (vertex, DFA state) |
| at most k edges / exactly k transfers | layered Bellman-Ford dp[i][v]; **copy** the previous layer or you silently allow more than k |
| paths of length exactly k, k <= 1e18 | adjacency matrix power; min-plus for the shortest such walk |
| minimise cost **per step** / a ratio around a cycle | min mean cycle: binary search lambda, subtract from all edges, test for a negative cycle |
| maximise profit - t*cost | Dinkelbach / fractional programming — binary search t, test the sign |
| expected moves / probability with cycles | linear system + Gauss; **on a tree** write E[v] = a*E[par] + b and eliminate bottom-up in O(n) |
| shortest cycle (girth) | BFS from every vertex; through a fixed edge = delete it, shortest path between its ends |

## FLOW AND MATCHING — more

| you see | reach for |
|---|---|
| each edge must carry **at least** L | flow with lower bounds: set f=L, route excess via super source/sink, add t to s with capacity inf |
| no source/sink, everything balances | circulation with demands — same construction, no s/t |
| k vertex- or edge-disjoint paths, general k | Menger = max flow with unit caps; split vertices for vertex-disjoint |
| matching in a **general** graph (odd cycles) | Blossom O(n^3); size via Tutte-Berge |
| "nobody would rather swap" — stable | Gale-Shapley O(n^2); the **proposing** side gets its optimum |
| colour edges, none sharing a vertex | Konig edge colouring: exactly Delta on bipartite (Vizing: Delta or Delta+1 general) |
| min cut with **no s and t given** | Stoer-Wagner O(n^3); also min cut <= min degree |
| min cut between **many pairs** | Gomory-Hu tree: n-1 max flows, answer = min edge on the tree path |
| subgraph maximising edges over vertices | densest subgraph: binary search the density + min cut |
| grid/planar "block every left-right path" | **planar duality**: min cut = shortest path in the dual (8-adjacency in grids) |
| orient edges so in-degree <= k | bipartite flow, edge-nodes to vertex-nodes with capacity k |
| listed vertices odd degree, rest even | T-join / cycle space over GF(2) — **never** a flow |

## MST AND SPANNING — more

| you see | reach for |
|---|---|
| "min possible max weight on a route", "smallest w making u,v connected", "everything reachable using edges <= w" | **Kruskal reconstruction tree** — node per merge, LCA value = bottleneck, "reachable with weight <= w" becomes a SUBTREE |
| reach everything **from the capital**, one-way | Chu-Liu / Edmonds arborescence — Kruskal is simply wrong |
| complete graph, weights from a formula (xor, abs diff, Manhattan) | **Boruvka** + a structure giving the cheapest edge leaving a component |
| is edge e in **some** / **every** MST | some: endpoints in different components before its weight class; every: it is a bridge within that class |
| connect k special cities, others optional | **Steiner tree DP** dp[mask][v], O(3^k n + 2^k(m + n log n)), k <= ~12 |

## SPECIAL GRAPH CLASSES — the class IS the gift

| you see | the class, and what it buys |
|---|---|
| "every pair played exactly once" | **tournament**: a Hamiltonian path always exists (O(n log n) insertion); strongly connected implies a Hamiltonian cycle; a "king" always exists. The answer is never -1 |
| "intervals conflict if they overlap" | **interval graph**: colouring = max point overlap, MIS = greedy by right endpoint, max clique = max overlap. All sweeps — never build the graph |
| max clique / MIS, n <= 40 or sparse | meet in the middle 2^(n/2); degeneracy (k-core) ordering; Bron-Kerbosch with pivot. clique <= degeneracy + 1 |
| count triangles / common neighbours per edge | orient **low degree to high degree** and intersect lists: O(m sqrt m) |
| the problem is about edges being adjacent to each other | **line graph**: edge colouring becomes vertex colouring, Euler in G becomes Hamiltonian in L(G). L(G) has sum(deg^2)/2 edges — explodes on stars |
| "reach t in an even number of steps" | **bipartite double cover**: duplicate to (v,0)/(v,1) |
| walk every road at least once and return | **Chinese postman**: pair odd-degree vertices by shortest paths + min-weight matching (bitmask if <= 20 odd) |
| cyclic string containing every length-k word once | **de Bruijn**: Eulerian circuit on (k-1)-mers |
| "does a graph exist with these degrees" | Erdos-Gallai (test) / Havel-Hakimi (construct) — **simple graphs only** |
| edges appear/vanish **online** plus path queries | link-cut tree (Euler tour tree if connectivity only). Deletions only? Reverse time and use plain DSU |

## TRANSFORMATIONS — this is secretly that

| you have | it is really | tell |
|---|---|---|
| "sort using allowed swaps (i,j)" | components of the swap graph; min swaps = n - number of cycles | the swap set is a graph |
| an operation coupling row i with column j | bipartite graph rows against columns, each cell an edge | answers like 2^(n+m-components) |
| x_i / x_j = k, or x_i - x_j = d, consistency | weighted DSU with potentials | contradiction iff a cycle with nonzero total |
| "minimum rooms / platforms / machines" | max point overlap = min chain cover | sweep, do not build the graph |
| "min rounds with prerequisites, unit time" | longest path in the DAG = topological level | with durations it is the critical path |
| "place items, no two share a row or column" | bipartite matching rows against columns | a rook placement IS a matching |
| a full pairwise **distance matrix**, reconstruct the tree | MST of the matrix, then verify all distances | distance-matrix problems are MST plus validation |
| "remove redundant edges" / "is this a valid distance matrix" | transitive reduction: (i,j) redundant iff some k has d[i][k]+d[k][j] <= d[i][j] | one Floyd pass answers both |
| state = at most 20 booleans, a move flips a fixed set | **GF(2) linear algebra**, not BFS | reachability and min-ops both come from the basis |
| "make all values equal by moving units between neighbours" | min-cost flow as the MODEL; on a line/tree it collapses to prefix sums | recognise the flow to prove the greedy, then do not implement it |
| two or three tokens moving at once, small n | BFS on the product graph over (u,v) or (u,v,w) | n^2 or n^3 states is the intended solution |
| "k centers minimising the maximum distance" | binary search the radius plus greedy cover | the phrase "minimise the maximum" |
| "add edge" events, later "connected at time t / weight <= w" | Kruskal reconstruction tree — threshold queries become subtree queries | any monotone-connectivity family |
| "minimum arrows stabbing all intervals" | greedy by right endpoint | complement of interval scheduling |
| "letter a becomes letter b" rewriting | functional graph / DSU on 26 nodes | alphabet size is the graph size, not the string length |
| all-pairs shortest path, n <= 500, edges get added | Floyd **incremental**: one O(n^2) pass per new edge | full recompute is the trap |
| "max weight subset, no two adjacent" | the class decides: tree to DP, interval to sweep, bipartite to Konig, general is NP-hard | — |

## TRAPS

- **Floyd**: k outermost. d[i][i]=0, min over multi-edges, skip relaxing from INF.
- **Dijkstra**: keep the stale check. Invalid with ANY negative edge, even without a negative cycle.
- **0-1 BFS**: no visited-on-push; keep the `d[u]+w < d[v]` test.
- **Bellman-Ford** only finds cycles reachable from the source. For "does any negative cycle exist", add a virtual source with 0-edges to every vertex.
- **Two-BFS diameter** is valid on trees with non-negative weights only.
- **Bridge** is `low[v] > tin[u]` (strict); **articulation** is `low[v] >= tin[u]`. Skip the parent by EDGE ID, or parallel edges are misreported as bridges.
- **Block-cut is not the bridge tree.** Vertex removal to block-cut. Edge removal to bridge tree.
- **Eulerian** needs the degree condition PLUS one component among edge-bearing vertices. Hierholzer must be iterative with a persistent per-vertex iterator, else O(E^2).
- **Min cut recovery**: the cut is what is reachable from s in the RESIDUAL graph. Reachable-from-s and not-reaching-t are two DIFFERENT min cuts; "in some min cut" is not "in every min cut".
- **Konig and "MIS = n - matching" are bipartite only.** Dilworth needs a real poset — take the transitive closure first.
- **Min path cover** counts vertex-disjoint paths; on a cyclic graph it is NP-hard.
- **MST weight is unique, the tree is not.** Counting MSTs is Kirchhoff per weight class on the contracted graph.
- **Centroid is not the center.** Centroid balances sizes; center minimises eccentricity. Both can be two adjacent vertices.
- **Small-to-large**: iterate the SMALLER container into the larger, then swap. Backwards is O(n^2) and looks identical.
- **Tree knapsack**: cap the loop by the merged subtree size.
- **Lexicographic topo order**: "smallest order" is min-heap Kahn. "Each label's position smallest" is reverse graph plus max-heap, then reverse. Different answers.
- **Flow with lower bounds** is feasible only if the auxiliary flow SATURATES ALL lower-bound edges.
- **Dinic is already O(E sqrt V)** on unit-capacity / matching networks; Hopcroft-Karp buys nothing there.
- **Bipartiteness reasons about the undirected graph** — it says nothing about a digraph's reachability parity.
