// ============================================================================
// CARTESIAN TREE -- build in O(n) with a monotonic stack
// ----------------------------------------------------------------------------
// A binary tree over the array where the ROOT of any range is that range's
// minimum (or maximum), and the two halves recurse to its left and right.
// In-order traversal returns the original array; the heap property is on
// value. It is the divide-and-conquer recursion tree of "split at the extreme"
// made explicit, in linear time.
//
// The build: sweep left to right with a stack of the RIGHTMOST SPINE. For a
// new element, pop everything larger -- the last popped becomes its left
// child, and the new node becomes the right child of whatever survives.
//
// Why it matters: it turns "sum over all subarrays of f(min)" from a quadratic
// enumeration into a per-node contribution, and it gives O(1) RMQ via LCA.
//
// ############################################################################
// #  TIES DECIDE WHETHER YOU DOUBLE COUNT
// #
// #  With equal values, `<` and `<=` in the pop test produce DIFFERENT trees,
// #  both valid heaps. For "sum over subarrays of the minimum" that choice is
// #  the difference between counting a subarray once and counting it twice.
// #
// #  Fix it by breaking ties on INDEX -- treat (value, index) as the key.
// #  Below: pop while `a[st.back()] > a[i]` (strict), so among equals the
// #  LEFTMOST wins and every subarray is charged to exactly one node.
// ############################################################################
//
// PITFALLS:
//   The stack holds INDICES, not values -- you need the index for the ranges.
//   Each node's range is exactly (previous smaller, next smaller) exclusive.
//     That is the same pair DS/monotonic_stack.cpp computes; if all you need
//     is the contribution, that file is shorter and you can skip the tree.
//   Recursion over the built tree can be n deep on a sorted array. Iterate,
//     or process nodes in decreasing value order.
//   For a MAX-Cartesian tree flip the comparison; do not negate the values if
//     they can be LLONG_MIN.
//   `root` is whatever is left at the bottom of the stack, not index 0.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   sum over all subarrays of their minimum         contribution: each node
//                                                   owns (i-l)*(r-i) subarrays
//   ... of max, gcd-like, or any "extreme"          same tree, same formula
//   count subarrays where a[i] is the minimum       (i-l)*(r-i)
//   largest rectangle in a histogram                node value * range width
//   O(1) RMQ                                        LCA on this tree
//   "the D&C splits at the range minimum"           this IS that recursion
//                                                   tree; build it and the
//                                                   D&C becomes a tree DP
//   suffix tree structure from SA + LCP             Cartesian tree of the LCP
//                                                   array -- its internal
//                                                   nodes are the LCP
//                                                   intervals
//   treap with given priorities                     a Cartesian tree on
//                                                   (key, priority)
//   "for each element, the range it dominates"      the (l, r) arrays below
//
// NOT THIS:
//   you only need previous/next smaller -> DS/monotonic_stack.cpp, no tree.
//   the array changes -> rebuild is O(n); there is no update operation here.
//   you need range min for arbitrary ranges and nothing else -> sparse table.
// ============================================================================
int n;
ll a[200005];
int L[200005], R[200005], root_;   // children, -1 if none

// MIN-Cartesian tree. Ties: leftmost equal value becomes the ancestor.
void build() {
    for (int i = 0; i < n; i++) L[i] = R[i] = -1;
    vector<int> st;
    for (int i = 0; i < n; i++) {
        int last = -1;
        while (!st.empty() && a[st.back()] > a[i]) last = st.back(), st.pop_back();
        L[i] = last;                        // everything popped hangs left
        if (!st.empty()) R[st.back()] = i;  // we become the survivor's right
        st.push_back(i);
    }
    root_ = st.empty() ? -1 : st[0];
}

////////////////////////////////////////////////////////////////////////////////
// THE RANGE EACH ELEMENT OWNS -- usually all you need, no tree required.
//
//   lo[i] = index of the previous element strictly smaller   (-1 if none)
//   hi[i] = index of the next element smaller-or-equal        (n if none)
//
// The asymmetric strict/non-strict pair is what makes equal values partition
// the subarrays instead of sharing them. a[i] is the minimum of exactly
// (i - lo[i]) * (hi[i] - i) subarrays.
////////////////////////////////////////////////////////////////////////////////
int lo[200005], hi[200005];

void ranges() {
    vector<int> st;
    for (int i = 0; i < n; i++) {
        while (!st.empty() && a[st.back()] >= a[i]) st.pop_back();
        lo[i] = st.empty() ? -1 : st.back();
        st.push_back(i);
    }
    st.clear();
    for (int i = n - 1; i >= 0; i--) {
        while (!st.empty() && a[st.back()] > a[i]) st.pop_back();
        hi[i] = st.empty() ? n : st.back();
        st.push_back(i);
    }
}

// sum over every subarray of its minimum
ll sum_of_subarray_mins() {
    ranges();
    ll tot = 0;
    for (int i = 0; i < n; i++)
        tot += a[i] * (ll)(i - lo[i]) * (ll)(hi[i] - i);
    return tot;
}

// largest rectangle in a histogram -- the same two arrays
ll largest_rectangle() {
    ranges();
    ll best = 0;
    for (int i = 0; i < n; i++)
        best = max(best, a[i] * (ll)(hi[i] - lo[i] - 1));
    return best;
}
