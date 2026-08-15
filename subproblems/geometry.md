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
| "is p expressible as a + b, a in P, b in Q" for convex P, Q | **Minkowski sum**, O(\|P\|+\|Q\|), then point-in-convex-polygon O(log n) | the sum of convex sets is convex |
| distance between two convex polygons / do they collide | Minkowski sum of P and -Q, then distance from the origin | reduces a pair problem to a single polygon |
| intersect two convex polygons | two-pointer merge O(n+m), or half-plane intersection of all edges | — |
| extreme point in a direction / tangents from an external point | ternary or binary search on the hull, O(log n) | enables per-query hull work |
| smallest circle covering all points | **Welzl**, expected O(n) | determined by 2 or 3 boundary points |
| smallest circle covering >= k points | angular sweep of a fixed-radius circle around each point, O(n^2 log n) | "radius R, maximise covered points" |
| nearest neighbour / per-point closest other | KD-tree | when you need per-point answers, not just the global closest pair |
| Euclidean MST | **Delaunay** triangulation, then MST on its O(n) edges | EMST is a subset of Delaunay |
| largest empty circle, farthest-point queries | **Voronoi** — candidates are Voronoi vertices | dual of Delaunay |
| Manhattan MST | 8-octant nearest-neighbour sweep, O(n log n) | only one candidate edge per octant is ever needed |
| area of a union of triangles / polygons | vertical decomposition into slabs; or signed-edge cancellation | avoids computing the union boundary |
| area of a union of circles | Green's theorem over arcs, or Simpson over x-slices | arc integration is exact |
| any continuous 1D geometric integral | **adaptive Simpson** | needs an even interval count and a depth cap |
| maximise a convex geometric quantity over one real parameter | ternary search with a FIXED iteration count (~200) | never loop on epsilon |
| 3D volume / hull | incremental 3D hull O(n^2); polyhedron volume = sum of mixed products / 6 | — |
| latitude and longitude in the input | great-circle / spherical distance | — |
| do any two of n segments intersect | sweep line with a BST ordered by y-at-current-x | adjacent-in-BST pairs are the only candidates |
| which polygon contains each of q query points | persistent segment tree over vertical slabs, O(log n) | offline sweep also works |
| "how many regions do these segments create" | sort edges by angle at each vertex, walk next-edge to trace faces; Euler `V - E + F = 2` | — |
| sorting points by angle | `half(p)` (upper vs lower) then cross product — **never atan2** | exact, fast, no precision loss |
| fix a point, sweep a ray | radial / angular sweep, O(n^2 log n) | turns an O(n^3) triangle count into O(n^2 log n) |
| clip a polygon by a half-plane | **Sutherland-Hodgman** | the building block of convex intersection |
