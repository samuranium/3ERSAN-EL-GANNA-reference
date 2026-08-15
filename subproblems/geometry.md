# GEOMETRY — subproblems

**The code lives in the other team's `Geomtry.pdf` and `Rules Geomtry.pdf`,
not in this repo.** Entries route to a section there.

## PDF CONTENTS

`Geomtry.pdf`: **1.1** All pairs · **1.2** Closest pair problem ·
**1.3** Rectangle Union · **2.1** QRQ4 · **3.1** points · **3.2** Lines ·
**3.3** Triangles · **3.4** Circles · **3.5** Polygons · **3.6** Closest pair ·
**3.7** Convex Hull · **3.8** Convex hull monotone chain ·
**3.9** Half plane intersections

`Rules Geomtry.pdf`: formulas — areas, angles, distances, triangle centres.

## BY QUESTION

| you see | do this | where |
|---|---|---|
| which side of a line is P on | cross product sign | §3.1 points, §3.2 Lines |
| do two segments intersect | orientation tests ×4 + collinear case | §3.2 |
| distance point→line / point→segment | projection, clamp to the segment | §3.2 |
| area of a polygon | shoelace, /2, take \|·\| | §3.5 Polygons |
| is a point inside a polygon | ray casting, or winding number | §3.5 |
| is a point inside a **convex** polygon | binary search on the fan, O(log n) | §3.5 |
| smallest convex shape containing points | convex hull | §3.7, §3.8 |
| diameter of a point set | hull + rotating calipers | §3.7 |
| two closest points | divide & conquer, or sweep | §1.2, §3.6 |
| area covered by rectangles | sweep line + segment tree | §1.3 Rectangle Union |
| intersection of half planes | §3.9 | §3.9 |
| max points on one line | group by slope from each point, O(n² log n) | — |
| circle intersections, tangents | §3.4 Circles | §3.4 |
| triangle centres, circumradius | §3.3 Triangles, `Rules Geomtry.pdf` | — |
| lattice points on a segment | `gcd(dx, dy) + 1` | `math/extgcd.cpp` |
| lattice points inside a polygon | Pick: A = I + B/2 − 1 | `Rules Geomtry.pdf` |

## MANHATTAN / CHEBYSHEV

The other team keeps this as a separate printed sheet (`Manhattan Trick.pdf`).

| you see | do this |
|---|---|
| Manhattan distance \|x1−x2\|+\|y1−y2\| | rotate: `(x+y, x−y)` turns it into Chebyshev `max(\|dx\|,\|dy\|)` |
| Chebyshev distance | rotate back: `((u+v)/2, (u−v)/2)` |
| farthest pair under Manhattan | rotate, then it is max−min per coordinate |
| Manhattan MST | 4 (or 8) sweeps by octant, then Kruskal |
| d dimensions | 2^(d−1) sign combinations of `±x₁±x₂…`; take the max |

## RECOGNITION — when a "geometry" problem is not geometry

| the statement | it is really |
|---|---|
| points on a line, intervals | sorting / sweep, not geometry |
| axis-aligned rectangles only | sweep line + segment tree |
| grid movement | BFS on a graph |
| "closest pair" with small coordinates | bucket by cell |
| distances that are all Manhattan | the rotation above, then 1D |
| angles used only for sorting | `atan2` once, or compare by half-plane + cross |

## TRAPS

- **Use integers.** Cross products and dot products of integer inputs stay
  integral; converting to double throws away the exactness that makes the
  orientation test reliable. Only divide at the very end.
- Cross products overflow: coordinates up to 1e9 give products up to 1e18 —
  `ll` is exactly enough, `int` is not.
- `atan2` comparisons are not reliable for sorting by angle; compare by
  half-plane first, then by cross product.
- Convex hull: decide up front whether collinear points on the hull edge are
  kept or dropped. Both conventions appear and they give different answers.
- Degenerate cases are where the marks are: zero-length segments, duplicate
  points, all points collinear, n = 1 and 2.

---

# RESEARCH ADDITIONS


## GEOMETRY

| you see | reach for | the tell |
|---|---|---|
| "is p expressible as a + b, a in P, b in Q" for convex P, Q | **Minkowski sum** — NOT IN THE REPO OR THE PDF. Normalise both to CCW starting at the bottom-most-then-left-most vertex, then merge the edge vectors by angle. Point-in-convex-polygon (§3.5) answers the query | the sum of convex sets is convex |
| distance between two convex polygons / do they collide | Minkowski sum of P and -Q, then distance from the origin | reduces a pair problem to a single polygon |
| intersect two convex polygons | half-plane intersection of all edges — PDF §3.9 | — |
| extreme point in a direction / tangents from an external point | binary search on the hull from §3.7 / §3.8, O(log n) | enables per-query hull work |
| smallest circle covering all points | **Welzl** — NOT IN THE REPO OR THE PDF. Shuffle, then recurse: add points one at a time, and when one falls outside, rebuild with it forced onto the boundary. Expected O(n) | determined by 2 or 3 boundary points |
| smallest circle covering >= k points | angular sweep of a fixed-radius circle around each point, O(n^2 log n) — NOT IN THE REPO | "radius R, maximise covered points" |
| nearest neighbour / per-point closest other | KD-tree — NOT IN THE REPO. For the GLOBAL closest pair only, the PDF has it: §1.2 / §3.6 | when you need per-point answers, not just the global closest pair |
| Euclidean MST | **Delaunay** triangulation then MST — NOT IN THE REPO OR THE PDF, and long. For n <= ~2000 just build all n^2 edges and run Kruskal (`graph/MST/kruskal with dsu.cpp`) | EMST is a subset of Delaunay |
| largest empty circle, farthest-point queries | **Voronoi** — NOT IN THE REPO OR THE PDF. Candidates are Voronoi vertices; for small n, binary search the radius and test candidate centres directly | dual of Delaunay |
| Manhattan MST | 8-octant nearest-neighbour sweep to get O(n) candidate edges, then Kruskal. The sweep is NOT IN THE REPO; the general "MST from a weight formula" pattern is `graph/MST/boruvka.cpp` | only one candidate edge per octant is ever needed |
| area of a union of triangles / polygons | vertical decomposition into slabs, or signed-edge cancellation — NOT IN THE REPO. For axis-aligned RECTANGLES the PDF has it: §1.3 Rectangle Union | avoids computing the union boundary |
| area of a union of circles | Green's theorem over arcs — NOT IN THE REPO. Circle primitives are PDF §3.4 | arc integration is exact |
| any continuous 1D geometric integral | **adaptive Simpson** — NOT IN THE REPO, but it is 15 lines: `simpson(a,b) = (b-a)/6 * (f(a) + 4f(m) + f(b))`, recurse on both halves while `|whole - (left+right)| > 15*eps`, with a hard depth cap | split at known discontinuities first, or it recurses forever |
| maximise a convex geometric quantity over one real parameter | ternary search with a FIXED iteration count (~200) | never loop on epsilon |
| 3D volume / hull | incremental 3D hull O(n^2) — NOT IN THE REPO. Polyhedron volume is just `sum of mixed products / 6` and needs no hull | — |
| latitude and longitude in the input | great-circle distance — the formula is in `Rules Geomtry.pdf` | — |
| do any two of n segments intersect | Bentley-Ottmann sweep with a BST ordered by y-at-current-x — NOT IN THE REPO. n <= ~2000? check all pairs with the §3.2 segment test | adjacent-in-BST pairs are the only candidates |
| which polygon contains each of q query points | persistent segment tree over vertical slabs (`DS/segment tree/persistent/`), O(log n); the slab decomposition itself is NOT IN THE REPO | offline sweep also works |
| "how many regions do these segments create" | Euler's formula `V - E + F = 2` is usually enough on its own; full face tracing (sort edges by angle at each vertex, walk next-edge) is NOT IN THE REPO | — |
| sorting points by angle | `half(p)` (upper vs lower) then cross product — **never atan2**. Cross product is PDF §3.1 | exact, fast, no precision loss |
| fix a point, sweep a ray | radial / angular sweep, O(n^2 log n) | turns an O(n^3) triangle count into O(n^2 log n) |
| clip a polygon by a half-plane | **Sutherland-Hodgman** — NOT IN THE REPO, but it is 12 lines: walk the edges, emit the vertex if it is inside, and emit the crossing point whenever an edge changes side. PDF §3.9 uses it | the building block of convex intersection |
