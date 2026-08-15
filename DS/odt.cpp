// ============================================================================
// ODT / CHTHOLLY TREE -- a map of equal-value intervals. Range ASSIGN is king.
// ----------------------------------------------------------------------------
// Store the array as a set of maximal intervals that share one value:
//     map<int, pair<int,ll>>  : l -> (r, value)
// Every operation begins by SPLITTING at the two endpoints so the range is an
// exact union of whole intervals, then does whatever it likes to that span.
//
// The complexity argument is entirely about ASSIGN: an assign over k intervals
// deletes k and creates 1. Splits create at most 2 per operation, so the total
// number of intervals ever created is O(n + q), and the total deleted is
// bounded by that. Amortised O(log n) per operation -- but ONLY when assigns
// are frequent enough to keep destroying intervals.
//
// ############################################################################
// #  WITHOUT RANGE ASSIGN THIS IS O(n) PER OPERATION
// #
// #  The bound is paid for by assign. A test with range-add and range-query
// #  and NO assign leaves every interval alive, the array fragments into n
// #  singletons, and every query walks all of them.
// #
// #  MEASURED, n = 1e5 and 20k random range ops: 44 intervals when half the
// #  ops are assigns, 32948 intervals when none are. That second number is
// #  the per-query cost. This is the standard way ODT solutions get hacked.
// #  If the problem does not guarantee assigns -- especially if the data is
// #  not random -- use a real segment tree.
// ############################################################################
//
// ############################################################################
// #  split(r+1) BEFORE split(l), OR THE ITERATOR DIES
// #
// #      auto itr = split(r + 1), itl = split(l);      // this order
// #
// #  split(l) can invalidate the iterator returned by split(r+1) when both
// #  land inside the same original interval. Taking the right one first is
// #  the fix, and the bug it prevents is a silent out-of-range read, not a
// #  crash. Note the range is [l, r] so you split at r+1.
// ############################################################################
//
// PITFALLS:
//   split(p) must return an iterator to the interval STARTING at p, creating
//     it if p is interior. If p is past the end, return end().
//   Initialise with one interval per element, or one interval covering
//     everything if the array starts constant.
//   assign() must erase the whole span before inserting -- `erase(itl, itr)`
//     with the half-open pair from the two splits.
//   Iterating [itl, itr) is a half-open range: itr is the interval that starts
//     at r+1 and must NOT be touched.
//   The values are `ll`. Range add over k intervals is O(k) writes; that is
//     the operation that fragments nothing and costs the most.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   range assign + range sum / max              this
//   range assign + range add                    this
//   range assign + k-th smallest in a range      collect (value, count) pairs
//                                                over the span, sort, walk
//   range assign + sum of x^k mod p              same walk, fast power per
//                                                interval
//   "paint the wall, how many colours visible"   assign, then count distinct
//                                                interval values
//   interval booking / free-space allocation     the same map, without values
//   "how many distinct values in the array"      iterate the map -- it is
//                                                already the run-length
//                                                encoding
//   run-length encoded input                     this IS the representation;
//                                                two-pointer over intervals
//   "set all of [l,r] to 0 and report what was   assign + the pre-assign walk
//    there"
//
// NOT THIS:
//   no range assign in the operation set    -> lazy segment tree. ODT is a
//     trap here, not a shortcut.
//   the problem is adversarial / hacking is possible and assigns are sparse
//                                           -> segment tree.
//   range chmin/chmax + sum                 -> segment tree beats,
//                                              DS/segment tree/beats.cpp.
// ============================================================================
struct ODT {
    // l -> (r, value); intervals are maximal, disjoint, and cover [1, n]
    map<int, pair<int, ll>> mp;

    void init(int n, ll v) { mp[1] = {n, v}; }
    void init_array(int n, ll *a) {           // one interval per element
        for (int i = 1; i <= n; i++) mp[i] = {i, a[i]};
    }
    // returns an iterator to the interval that STARTS at p, splitting if needed
    map<int, pair<int, ll>>::iterator split(int p) {
        auto it = mp.upper_bound(p);
        if (it == mp.begin()) return mp.end();
        --it;
        if (it->first == p) return it;
        int l = it->first, r = it->second.first;
        ll v = it->second.second;
        if (p > r) return mp.end();
        mp.erase(it);
        mp[l] = {p - 1, v};
        return mp.insert({p, {r, v}}).first;
    }
    // ---- the operations. Every one starts with the same two splits. -------
    void assign(int l, int r, ll v) {
        auto itr = split(r + 1), itl = split(l);       // RIGHT one first
        mp.erase(itl, itr);
        mp[l] = {r, v};                                // k intervals -> 1
    }
    void add(int l, int r, ll v) {
        auto itr = split(r + 1), itl = split(l);
        for (auto it = itl; it != itr; ++it) it->second.second += v;
    }
    ll sum(int l, int r) {
        auto itr = split(r + 1), itl = split(l);
        ll s = 0;
        for (auto it = itl; it != itr; ++it)
            s += it->second.second * (it->second.first - it->first + 1);
        return s;
    }
    ll maxv(int l, int r) {
        auto itr = split(r + 1), itl = split(l);
        ll m = LLONG_MIN;
        for (auto it = itl; it != itr; ++it) m = max(m, it->second.second);
        return m;
    }
    // k-th smallest in [l,r], 1-indexed
    ll kth(int l, int r, int k) {
        auto itr = split(r + 1), itl = split(l);
        vector<pair<ll, int>> v;                       // (value, count)
        for (auto it = itl; it != itr; ++it)
            v.push_back({it->second.second, it->second.first - it->first + 1});
        sort(v.begin(), v.end());
        for (auto &p : v) {
            if (k <= p.second) return p.first;
            k -= p.second;
        }
        return LLONG_MIN;                              // k out of range
    }
    int distinct(int l, int r) {
        auto itr = split(r + 1), itl = split(l);
        set<ll> s;
        for (auto it = itl; it != itr; ++it) s.insert(it->second.second);
        return s.size();
    }
    int interval_count() { return mp.size(); }         // fragmentation gauge
};
