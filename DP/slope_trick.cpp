// ============================================================================
// SLOPE TRICK -- maintain a convex piecewise-linear f(x) with two heaps
// ----------------------------------------------------------------------------
// The DP value is a FUNCTION of one parameter, and that function is convex,
// piecewise linear, continuous. Rather than an array over x, store:
//
//     minf          the minimum value of f
//     L (max-heap)  the slope-change points to the LEFT of the flat bottom
//     R (min-heap)  the slope-change points to the RIGHT
//
// Every breakpoint stored once per unit of slope change. The flat bottom is
// [L.top(), R.top()]. That is the entire representation -- f is recovered by
// walking outward, but you almost never need to.
//
// The operations, each O(log n):
//     f(x) += (x - a)+        "cost if you end above a"    -> add to R
//     f(x) += (a - x)+        "cost if you end below a"    -> add to L
//     f(x) += |x - a|         both of the above
//     f(x)  = min_{y<=x} f(y) prefix min  -> DROP R entirely (slope >= 0 gone)
//     f(x)  = min_{y>=x} f(y) suffix min  -> DROP L entirely
//     f(x) -> f(x - c)        shift right -> lazy add c to both heaps
//     f(x) -> min_{|y-x|<=c}  "slide"     -> shift L left by c, R right by c
//
// ############################################################################
// #  ADDING |x - a| IS A THREE-STEP DANCE, NOT TWO PUSHES
// #
// #      L.push(a); R.push(a);
// #      if (L.top() > R.top()) { swap the two tops; }
// #      minf += max(0LL, <the amount the bottom moved>);
// #
// #  Concretely: push a into L, then move L.top() into R; push a into R, then
// #  move R.top() into L. The minimum increases by |a - (old bottom)| only
// #  when a falls outside the flat region. Skipping the re-balance leaves the
// #  heaps interleaved and every later query is wrong, silently.
// ############################################################################
//
// ############################################################################
// #  BOTH HEAPS NEED LAZY SHIFT TAGS, AND APPLYING ONE ACROSS A SWAP IS THE BUG
// #
// #  Shifting f left/right is O(1) via a tag per heap. But when an element
// #  MOVES from L to R during a rebalance, its value must be un-tagged from
// #  L's frame and re-tagged into R's. Forgetting that is THE slope-trick
// #  bug: it passes every test where the two shifts happen to be equal.
// #  Below, addL / addR are those tags and push/pop always convert.
// ############################################################################
//
// PITFALLS:
//   The function MUST be convex. Adding any concave piece (a -|x-a|, a
//     "reward") destroys the invariant permanently. There is no check.
//   Only the MINIMUM is tracked exactly; f at a specific x needs a walk.
//   Prefix-min deletes the whole right heap. If you need it later, you cannot
//     -- that information is gone by design.
//   The classic "make the array non-decreasing with min total |change|" needs
//     only L and minf; R never grows. Use that as the sanity model.
//   Integer domain: all breakpoints are integers if the inputs are.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   min total |a_i - b_i| making b non-DECREASING     the canonical one --
//                                                     nondecreasing() below
//   ... non-increasing                                same, on the reversed
//                                                     array
//   ... strictly increasing                           subtract i from a_i
//                                                     first, then the above
//   "each step may move by at most c"                 the slide operation
//   "you may also pay to skip"                        add a constant-slope
//                                                     piece
//   min cost to equalise with a move budget            slide + |x-a|
//   convex cost merged UP A TREE                       slope trick + heap
//                                                      merging (small to
//                                                      large, or a leftist /
//                                                      pairing heap)
//   "exactly k pieces, cost convex in k"               aliens trick evaluates
//                                                      ONE tangent of this same
//                                                      function -- their PDF
//   min-cost flow on a line / regret greedy            the same convexity fact
//                                                      in a third vocabulary
//
// NOT THIS:
//   the cost is not convex          -> no slope trick. Full DP over x.
//   you need f at many x values     -> keep the array; the heaps only give the
//                                      minimum cheaply.
//   the parameter is a COUNT you must report -> aliens/Lagrangian, not this.
// ============================================================================
struct Slope {
    priority_queue<ll> L;                                   // max-heap
    priority_queue<ll, vector<ll>, greater<ll>> R;          // min-heap
    ll addL = 0, addR = 0;                                  // lazy shift tags
    ll minf = 0;

    void pushL(ll x) { L.push(x - addL); }
    void pushR(ll x) { R.push(x - addR); }
    ll  topL() const { return L.top() + addL; }
    ll  topR() const { return R.top() + addR; }
    void popL() { L.pop(); }
    void popR() { R.pop(); }

    // f(x) += (a - x)+     "penalty for finishing BELOW a"
    void add_left(ll a) {
        pushL(a);
        if (!R.empty() && topR() < a) {                     // rebalance
            ll t = topR(); popR();
            minf += a - t;
            pushR(a);                                       // re-tag into R
            popL(); pushL(t);                               // and t into L
        }
    }
    // f(x) += (x - a)+     "penalty for finishing ABOVE a"
    void add_right(ll a) {
        pushR(a);
        if (!L.empty() && topL() > a) {
            ll t = topL(); popL();
            minf += t - a;
            pushL(a);
            popR(); pushR(t);
        }
    }
    // f(x) += |x - a|
    void add_abs(ll a) { add_left(a), add_right(a); }

    // f(x) = min over y <= x of f(y)   -- the right slopes vanish
    void prefix_min() { while (!R.empty()) R.pop(); addR = 0; }
    // f(x) = min over y >= x of f(y)   -- the left slopes vanish
    void suffix_min() { while (!L.empty()) L.pop(); addL = 0; }

    // f(x) -> f(x - c): move the whole function right by c
    void shift(ll c) { addL += c, addR += c; }
    // f(x) -> min over |y - x| <= c of f(y): widen the flat bottom
    void slide(ll cl, ll cr) { addL -= cl, addR += cr; }

    ll min_value() const { return minf; }
};

// ---- the canonical use: min sum |a_i - b_i| with b non-decreasing ---------
// Each step: allow b_i to be anything >= the previous (prefix min), then pay
// |a_i - b_i|. Only the left heap ever matters, so this is also the smallest
// correct model to test the structure against.
ll min_cost_nondecreasing(vector<ll> &a) {
    Slope f;
    for (ll x : a) {
        f.prefix_min();       // b_i may be >= any earlier value: drop R
        f.add_abs(x);
    }
    return f.min_value();
}

// ---- non-decreasing, but each step may rise by at most `c` ---------------
ll min_cost_bounded_step(vector<ll> &a, ll c) {
    Slope f;
    for (ll x : a) {
        f.slide(0, c);        // b_i in [b_{i-1}, b_{i-1} + c]
        f.add_abs(x);
    }
    return f.min_value();
}
