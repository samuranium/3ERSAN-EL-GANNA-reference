// ============================================================================
// SEGMENT TREE BEATS -- range chmin/chmax WITH range sum, amortised O(n log n)
// ----------------------------------------------------------------------------
// `a[i] = min(a[i], x)` over a range is not a lazy tag: it acts differently on
// every element, so it cannot be composed or pushed down. Beats gets around it
// by storing enough per node to make MOST chmin calls O(1):
//
//     mx    the maximum in the node
//     mx2   the STRICT second maximum (-INF if all values are equal)
//     cmx   how many elements equal mx
//     sum
//
// The recursion has three cases:
//     x >= mx        nothing changes            -> return
//     mx2 < x < mx   only the maxima change     -> sum -= (mx-x)*cmx, mx = x
//     x <= mx2       cannot resolve here        -> RECURSE into both children
//
// The third case is the "beats" step. The potential argument (the number of
// distinct values in a subtree) bounds the total recursions at O(n log n) for
// chmin alone, O(n log^2 n) once range-add is in the mix.
//
// ############################################################################
// #  THE SECOND MAXIMUM MUST BE STRICT
// #
// #  mx2 is the largest value STRICTLY LESS than mx, not "the second element".
// #  With duplicates, a non-strict mx2 equals mx, the test `x <= mx2` fires
// #  when it should not, and the amortised bound collapses to O(n) per
// #  operation -- or the recursion never terminates. In merge, when the two
// #  children have equal mx, cmx ADDS and mx2 is max of the two mx2's.
// ############################################################################
//
// ############################################################################
// #  RANGE ADD NEEDS TWO LAZY TAGS, NOT ONE
// #
// #  An add applied to a node must shift the maxima and the non-maxima by
// #  DIFFERENT amounts, because a pending chmin has already clamped the
// #  maxima. Keep `add_mx` (applied to elements equal to mx) and `add_ot`
// #  (applied to everything else). One shared tag silently un-clamps values
// #  that chmin already lowered.
// #
// #  This file keeps chmin + chmax + sum WITHOUT range add -- that is the
// #  version worth having on paper. If the problem also needs range add,
// #  budget real time for it; it is a different, much longer structure.
// ############################################################################
//
// PITFALLS:
//   -INF for mx2 must survive arithmetic: use LLONG_MIN/4, never LLONG_MIN.
//   chmax is the exact mirror (mn, mn2, cmn). Having BOTH plus sum in one node
//     means push_down must apply four clamps in a consistent order.
//   The complexity is AMORTISED. It is not safe under rollback or persistence.
//   Do not "optimise" the second case by skipping the sum update -- that is
//     the only place the sum is maintained.
//   Query sum is a plain segment tree query; only the update is exotic.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   range chmin + range sum                      this
//   range chmin + range max                      this (max is free)
//   range chmax + range sum                      the mirror, chmax() below
//   both chmin and chmax + sum                   both node halves, four tags
//   "cap every value at x, then report the sum"  literally this
//   range assign + range sum                     do NOT use beats -- ODT or a
//                                                plain assign tag, DS/odt.cpp
//   count of elements equal to the max           cmx, already stored
//   "historic maximum" under range add           different structure: a
//                                                (current, historic) tag pair
//   range gcd with range add                     segment tree over the
//                                                difference array, not beats
//
// NOT THIS:
//   only chmin, and you never need the SUM  -> a plain max segment tree with a
//     min-tag works, because chmin composes fine when max is the only query.
//   the operation is range ASSIGN            -> assign IS a lazy tag. Beats is
//     wasted; see DS/odt.cpp or a normal lazy tree.
// ============================================================================
const ll NINF = LLONG_MIN / 4;

int n;
ll a[200005];

struct B { ll mx, mx2, sum; int cmx; };
B t[800005];

B mergeB(const B &L, const B &R) {
    B r;
    r.sum = L.sum + R.sum;
    if (L.mx == R.mx) {
        r.mx = L.mx, r.cmx = L.cmx + R.cmx;
        r.mx2 = max(L.mx2, R.mx2);
    } else if (L.mx > R.mx) {
        r.mx = L.mx, r.cmx = L.cmx;
        r.mx2 = max(L.mx2, R.mx);         // R's max is a strict runner-up
    } else {
        r.mx = R.mx, r.cmx = R.cmx;
        r.mx2 = max(R.mx2, L.mx);
    }
    return r;
}
void build(int v, int lo, int hi) {
    if (lo == hi) { t[v] = {a[lo], NINF, a[lo], 1}; return; }
    int mid = lo + hi >> 1;
    build(2 * v, lo, mid), build(2 * v + 1, mid + 1, hi);
    t[v] = mergeB(t[2 * v], t[2 * v + 1]);
}
// apply "clamp the maxima of this node down to x". Only legal when mx2 < x < mx.
void apply_min(int v, ll x) {
    if (x >= t[v].mx) return;
    t[v].sum -= (t[v].mx - x) * t[v].cmx;
    t[v].mx = x;
}
void push(int v) {
    apply_min(2 * v, t[v].mx);
    apply_min(2 * v + 1, t[v].mx);
}
void chmin(int v, int lo, int hi, int l, int r, ll x) {
    if (r < lo || hi < l || t[v].mx <= x) return;          // case 1: no-op
    if (l <= lo && hi <= r && t[v].mx2 < x) {              // case 2: O(1)
        apply_min(v, x);
        return;
    }
    push(v);                                               // case 3: beats
    int mid = lo + hi >> 1;
    chmin(2 * v, lo, mid, l, r, x);
    chmin(2 * v + 1, mid + 1, hi, l, r, x);
    t[v] = mergeB(t[2 * v], t[2 * v + 1]);
}
ll query_sum(int v, int lo, int hi, int l, int r) {
    if (r < lo || hi < l) return 0;
    if (l <= lo && hi <= r) return t[v].sum;
    push(v);
    int mid = lo + hi >> 1;
    return query_sum(2 * v, lo, mid, l, r) + query_sum(2 * v + 1, mid + 1, hi, l, r);
}
ll query_max(int v, int lo, int hi, int l, int r) {
    if (r < lo || hi < l) return NINF;
    if (l <= lo && hi <= r) return t[v].mx;
    push(v);
    int mid = lo + hi >> 1;
    return max(query_max(2 * v, lo, mid, l, r), query_max(2 * v + 1, mid + 1, hi, l, r));
}

////////////////////////////////////////////////////////////////////////////////
// chmax IS THE MIRROR. To get both in one tree, carry (mn, mn2, cmn) alongside
// and mirror every function. Below is the standalone chmax version -- paste
// whichever half the problem asks for rather than maintaining both.
////////////////////////////////////////////////////////////////////////////////
const ll PINF = LLONG_MAX / 4;

struct C { ll mn, mn2, sum; int cmn; };
C u[800005];

C mergeC(const C &L, const C &R) {
    C r;
    r.sum = L.sum + R.sum;
    if (L.mn == R.mn)      r.mn = L.mn, r.cmn = L.cmn + R.cmn, r.mn2 = min(L.mn2, R.mn2);
    else if (L.mn < R.mn)  r.mn = L.mn, r.cmn = L.cmn,        r.mn2 = min(L.mn2, R.mn);
    else                   r.mn = R.mn, r.cmn = R.cmn,        r.mn2 = min(R.mn2, L.mn);
    return r;
}
void buildC(int v, int lo, int hi) {
    if (lo == hi) { u[v] = {a[lo], PINF, a[lo], 1}; return; }
    int mid = lo + hi >> 1;
    buildC(2 * v, lo, mid), buildC(2 * v + 1, mid + 1, hi);
    u[v] = mergeC(u[2 * v], u[2 * v + 1]);
}
void apply_max(int v, ll x) {
    if (x <= u[v].mn) return;
    u[v].sum += (x - u[v].mn) * u[v].cmn;
    u[v].mn = x;
}
void pushC(int v) { apply_max(2 * v, u[v].mn), apply_max(2 * v + 1, u[v].mn); }

void chmax(int v, int lo, int hi, int l, int r, ll x) {
    if (r < lo || hi < l || u[v].mn >= x) return;
    if (l <= lo && hi <= r && u[v].mn2 > x) { apply_max(v, x); return; }
    pushC(v);
    int mid = lo + hi >> 1;
    chmax(2 * v, lo, mid, l, r, x), chmax(2 * v + 1, mid + 1, hi, l, r, x);
    u[v] = mergeC(u[2 * v], u[2 * v + 1]);
}
ll query_sumC(int v, int lo, int hi, int l, int r) {
    if (r < lo || hi < l) return 0;
    if (l <= lo && hi <= r) return u[v].sum;
    pushC(v);
    int mid = lo + hi >> 1;
    return query_sumC(2 * v, lo, mid, l, r) + query_sumC(2 * v + 1, mid + 1, hi, l, r);
}
