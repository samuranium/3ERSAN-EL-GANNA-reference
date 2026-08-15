// ============================================================================
// SUBMASK PARTITION DP -- O(3^n), the "split a set into groups" workhorse
// ----------------------------------------------------------------------------
// The shape: dp[mask] is built from dp[mask ^ sub] for every submask sub.
//
//     for (int mask = 1; mask < (1<<n); mask++)
//         for (int sub = mask; sub; sub = (sub-1) & mask)
//             dp[mask] = best(dp[mask], dp[mask ^ sub] + cost[sub]);
//
// Total work is sum over masks of 2^popcount(mask) = 3^n, because every bit is
// independently in `sub`, in `mask^sub`, or outside `mask`. n = 18 is 387M and
// too slow; n <= 16 (43M) is comfortable, n = 17 (129M) is borderline.
//
// This is NOT SOS DP. SOS answers "aggregate over submasks" for every mask in
// O(2^n * n) -- but only for an aggregate that a per-bit prefix sum can build
// (sum, max, count). A PARTITION needs `cost[sub]` combined with `dp[mask^sub]`,
// which is a convolution over the subset lattice, and no per-bit trick gives it.
//
// ############################################################################
// #  FIX THE LOWEST SET BIT TO KILL THE FACTOR OF 2
// #
// #  Every partition of `mask` into two parts is generated TWICE, once as
// #  (sub, mask^sub) and once as (mask^sub, sub). For MIN / MAX that is
// #  harmless -- just wasted time. For COUNTING it doubles the answer.
// #
// #  Force the lowest set bit of `mask` to live in `sub`:
// #      int low = mask & -mask;
// #      for (int sub = mask; sub; sub = (sub-1) & mask)
// #          if (sub & low) ...
// #  Halves the work and makes counting correct. See count_partitions().
// ############################################################################
//
// ############################################################################
// #  `sub = 0` IS NEVER EMITTED
// #
// #  `for (sub = mask; sub; sub = (sub-1) & mask)` stops before 0. That is
// #  what you want here -- an empty group is not a group -- but if you need
// #  the empty submask, use the do/while form in math/bitwise.cpp, or the
// #  transition silently never considers "take nothing".
// ############################################################################
//
// PITFALLS:
//   cost[] must be precomputed for all 2^n masks BEFORE the DP. Computing it
//     inside the loop turns 3^n into 3^n * (whatever cost costs).
//   `mask ^ sub` and `mask - sub` are the same only because sub is a submask.
//   dp[0] is the identity: 0 for min-cost/count-of-groups, 1 for counting
//     partitions, and dp[0] = 0 with everything else INF for min.
//   INF must survive the addition: use LLONG_MAX/4 or 1e18/4, never LLONG_MAX.
//   n up to 20 means 1<<20 arrays -- 8 MB per ll array. Two of them is fine,
//     five is not.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS -- everything below is this one loop with a different cost[]
// ----------------------------------------------------------------------------
//   partition n items into groups, minimise total cost   min_partition()
//   ... into EXACTLY k groups                            add a k dimension,
//                                                        O(3^n * k)
//   ... minimise the number of groups                    cost[sub] = 1 if sub
//                                                        is a legal group
//   minimum set cover (parts may OVERLAP)                min_cover() -- the
//                                                        subtly different loop
//   count the ways to partition into labelled groups     count_partitions()
//   graph colouring / chromatic number                   cost[sub] = 1 iff sub
//                                                        is independent --
//                                                        graph/graph_coloring.cpp
//   bin packing, n <= 16 items                           cost[sub] = 1 iff sub
//                                                        fits in one bin
//   split people into teams, team score given            cost[sub] = score[sub]
//   minimum number of rounds / days / trips              same as "minimise the
//                                                        number of groups"
//   "assign every task, each worker takes a SET"         one layer per worker,
//                                                        dp[i][mask]
//
// NOT THIS:
//   "assign task i to worker i", one item each  -> dp[mask] with popcount as
//     the row index, O(2^n * n). No submask loop. Cheaper by a huge factor.
//   "for each mask, sum over its submasks"      -> SOS DP, DP/sos.cpp.
//   "which sums are reachable"                  -> bitset, DS/bitset.cpp.
//   n > 20                                      -> not a bitmask problem.
// ============================================================================
const ll INF = 1e18 / 4;

int n;                       // number of items, n <= ~16
ll  cost[1 << 20];           // cost[sub] = cost of making `sub` ONE group,
                             // INF if `sub` is not a legal group
ll  dp[1 << 20];

// ---- minimum total cost to partition the full set into legal groups --------
// Groups are DISJOINT and cover everything. Returns INF if impossible.
ll min_partition() {
    int full = (1 << n) - 1;
    dp[0] = 0;
    for (int mask = 1; mask <= full; mask++) {
        dp[mask] = INF;
        int low = mask & -mask;                  // pin the lowest bit into sub
        for (int sub = mask; sub; sub = (sub - 1) & mask) {
            if (!(sub & low)) continue;          // each split seen once
            if (cost[sub] >= INF || dp[mask ^ sub] >= INF) continue;
            dp[mask] = min(dp[mask], dp[mask ^ sub] + cost[sub]);
        }
    }
    return dp[full];
}

// ---- number of ways to partition into UNLABELLED groups --------------------
// Pinning the low bit is what makes this a partition count and not a
// composition count -- without it every partition is counted k! times.
ll count_partitions() {
    int full = (1 << n) - 1;
    dp[0] = 1;
    for (int mask = 1; mask <= full; mask++) {
        dp[mask] = 0;
        int low = mask & -mask;
        for (int sub = mask; sub; sub = (sub - 1) & mask) {
            if (!(sub & low)) continue;
            if (cost[sub] >= INF) continue;      // reuse cost[] as legality
            dp[mask] += dp[mask ^ sub];
        }
    }
    return dp[full];
}

////////////////////////////////////////////////////////////////////////////////
// MINIMUM SET COVER -- parts may OVERLAP, so the transition is different.
// Do NOT pin the low bit here: `sub` is what the chosen set adds, and the
// same cover can legitimately be reached by different orders.
//
// `reach[s]` = the set of items some available group can cover, restricted
// to s. Classic form: pick the lowest UNCOVERED item and try every group
// containing it -- that keeps it O(2^n * #groups) instead of O(3^n).
////////////////////////////////////////////////////////////////////////////////
vector<int> groups;          // available groups, as masks

ll min_cover() {
    int full = (1 << n) - 1;
    vector<ll> d(full + 1, INF);
    d[0] = 0;
    for (int mask = 0; mask < full; mask++) {
        if (d[mask] >= INF) continue;
        int low = __builtin_ctz(~mask);          // lowest UNCOVERED item
        for (int g : groups) {
            if (!(g >> low & 1)) continue;       // must cover it, or we loop
            int nxt = mask | g;
            d[nxt] = min(d[nxt], d[mask] + 1);
        }
    }
    return d[full];
}
