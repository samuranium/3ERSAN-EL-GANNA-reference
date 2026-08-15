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
