// ============================================================================
// SWAG -- sliding window fold for ANY monoid, amortised O(1) per operation
// ----------------------------------------------------------------------------
// "Sliding Window Aggregation". A queue built from two stacks, each carrying
// PREFIX FOLDS of its own contents. The window's fold is one combine of the
// two stack tops.
//
//   back stack   push here,  keeps folds of "everything pushed since the flip"
//   front stack  pop  here,  keeps SUFFIX folds of the block it was given
//
// When the front stack empties, dump the whole back stack into it, recomputing
// folds in reverse. Every element moves between stacks at most once, so the
// dump is O(1) amortised.
//
// ############################################################################
// #  THIS IS WHAT A MONOTONIC DEQUE CANNOT DO
// #
// #  DS/monotonic_deque.cpp does min/max only -- it works by DISCARDING
// #  elements that can never win, which needs a total order. SWAG never
// #  discards, so it handles gcd, OR, AND, matrix product, "min and its
// #  count", modular sums -- any associative op with an identity.
// #
// #  If the op is INVERTIBLE (sum, xor), do not use either: two prefix
// #  arrays and a subtraction is shorter and faster.
// ############################################################################
//
// ############################################################################
// #  ORDER MATTERS -- combine(front, back), NEVER the reverse
// #
// #  The front stack holds the OLDER elements. For matrix products, string
// #  concatenation, function composition and min-plus, swapping the two
// #  arguments produces a different answer that is correct for every
// #  commutative test you will write. Check with a non-commutative op.
// ############################################################################
//
// PITFALLS:
//   `e` must be a true identity: combine(e, x) == x AND combine(x, e) == x.
//     0 for sum, 0 for gcd, LLONG_MAX for min, the identity matrix, "".
//   fold() on an empty window returns e -- decide whether that is meaningful
//     for your problem or an error.
//   Amortised, not worst case. A single pop can cost O(size). Fine for a loop
//     over the whole array; NOT fine if an adversary rewinds you.
//   Two pointers + SWAG is the standard "longest window where f(window)
//     satisfies P" pattern, and it needs P monotone in the window.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   sliding window min/max                       monotonic deque is shorter --
//                                                DS/monotonic_deque.cpp
//   sliding window gcd / AND / OR                this
//   sliding window matrix product / composition  this, order matters
//   sliding window "min and how many"            this, pair monoid
//   longest window with gcd == 1 (or > 1)        two pointers + this
//   longest window whose AND has a given bit     two pointers + this
//   count subarrays with gcd/AND/OR == k         the (value, count) stack
//                                                trick below is usually better
//   window product mod m, m not prime            this -- no inverse exists
//   fold over a queue in a BFS / simulation      this, verbatim
//
// NOT THIS:
//   invertible op (sum, xor, product mod a prime) -> prefix folds, O(1), no
//     structure at all.
//   the window is not a contiguous range         -> nothing here applies.
//   you need arbitrary range queries, not a window -> sparse table (idempotent)
//     or disjoint sparse table (any monoid), DS/sparse/.
// ============================================================================
struct SWAG {
    // ---- the monoid. Replace these two lines per problem. -----------------
    typedef ll T;
    static T combine(T a, T b) { return __gcd(a, b); }
    static T id() { return 0; }
    // -----------------------------------------------------------------------

    // each stack stores (value, fold of this stack from its bottom up to here)
    vector<pair<T, T>> back_, front_;

    bool empty() const { return back_.empty() && front_.empty(); }
    int  size()  const { return back_.size() + front_.size(); }

    void push(T x) {
        T f = back_.empty() ? x : combine(back_.back().second, x);
        back_.push_back({x, f});
    }
    void pop() {
        if (front_.empty()) {                     // flip: back -> front
            while (!back_.empty()) {
                T x = back_.back().first;
                back_.pop_back();
                T f = front_.empty() ? x : combine(x, front_.back().second);
                front_.push_back({x, f});         // suffix folds, reversed
            }
        }
        front_.pop_back();
    }
    T fold() const {                              // fold of the whole window
        if (front_.empty()) return back_.empty() ? id() : back_.back().second;
        if (back_.empty())  return front_.back().second;
        return combine(front_.back().second, back_.back().second);  // old, new
    }
};

// ---- longest subarray whose gcd is > 1, as the canonical two-pointer use ---
int longest_gcd_gt1(vector<ll> &a) {
    SWAG q;
    int best = 0, l = 0;
    for (int r = 0; r < (int)a.size(); r++) {
        q.push(a[r]);
        while (!q.empty() && q.fold() == 1) q.pop(), l++;
        best = max(best, r - l + 1);
    }
    return best;
}

////////////////////////////////////////////////////////////////////////////////
// THE OTHER TOOL FOR THE SAME FAMILY: stack of (value, count) per right end.
//
// gcd / AND / OR over all suffixes ending at r takes only O(log C) DISTINCT
// values, because each step either keeps the value or strictly loses a bit.
// Keep them as runs. This answers "count subarrays whose gcd == k" for ALL k
// in O(n log C) -- something the sliding window cannot do, because the
// property is not monotone in a two-pointer sense.
////////////////////////////////////////////////////////////////////////////////
ll count_subarrays_with_gcd(vector<ll> &a, ll k) {
    vector<pair<ll, ll>> cur;                     // (gcd value, how many l's)
    ll total = 0;
    for (ll x : a) {
        vector<pair<ll, ll>> nxt;
        nxt.push_back({x, 1});
        for (auto &p : cur) {
            ll g = __gcd(p.first, x);
            if (g == nxt.back().first) nxt.back().second += p.second;
            else nxt.push_back({g, p.second});
        }
        cur.swap(nxt);
        for (auto &p : cur) if (p.first == k) total += p.second;
    }
    return total;
}
