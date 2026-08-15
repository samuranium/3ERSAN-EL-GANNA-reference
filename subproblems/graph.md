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
| bipartite matching | Kuhn | `graph/matching/kuhn.cpp` |
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
