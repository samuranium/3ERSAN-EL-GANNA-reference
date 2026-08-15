// ============================================================================
// SUBSET SUM BY BITSET -- feasibility in O(n * S / 64)
// ----------------------------------------------------------------------------
//     dp[0] = 1;   for each weight w:   dp |= dp << w;
// After the loop dp[s] is "some subset sums to exactly s". Two lines, and it
// turns an O(n*S) DP into O(n*S/64) -- n = 100, S = 1e5 becomes ~1.5e5 word
// operations.
//
// ############################################################################
// #  FEASIBILITY ONLY. NOT COUNTING, NOT MAXIMISING.
// #
// #  A bit is one bit: reachable or not. The moment the problem asks HOW MANY
// #  subsets, or the BEST value at that weight, you need a real DP array and
// #  the /64 is gone.
// #
// #    "can we make exactly S"          -> this
// #    "how many ways to make S"        -> dp[] of counts, O(n*S)
// #    "max value with weight <= S"     -> 0/1 knapsack, O(n*S)
// #    "fewest items summing to S"      -> BFS over reachable sums, or DP
// ############################################################################
//
// ############################################################################
// #  BOUNDED COUNTS: BINARY-SPLIT FIRST
// #
// #  k copies of weight w is NOT `dp |= dp << w` repeated k times when k is
// #  large. Split k into 1, 2, 4, ..., remainder and push each as one item:
// #      for (b = 1; k; b <<= 1) { t = min(b, k); push(t*w); k -= t; }
// #  That is O(log k) shifts instead of k, and it represents every count in
// #  [0, k] exactly once.
// #
// #  UNBOUNDED (any number of copies) is a different loop entirely -- a plain
// #  forward DP `for s: if (dp[s-w]) dp[s]=1`, which the bitset cannot express
// #  because each shift would only add one copy.
// ############################################################################
//
// PITFALLS:
//   MAXB is a compile-time constant and must exceed the largest sum you care
//     about. Bits above it are lost silently by the shift.
//   Negative weights: shift the whole domain by an OFFSET, work in
//     [0, 2*maxsum], and read answers back at offset + target.
//   Total sum can exceed the bitset even when the TARGET does not -- size by
//     the target, and skip items heavier than it.
//   Reconstruction needs the per-item snapshots (see below), which costs
//     O(n * S / 8) bytes. If n*S is large, recompute instead of storing.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   is sum S reachable                      reach()
//   which items give S                      recover() -- keeps snapshots
//   partition into two equal halves         total even, and reach(total/2)
//   minimise |A| - |B| over a partition     best reachable s near total/2
//   can each of q targets be made           one dp, then q lookups
//   k copies of each item                   binary splitting, see the box
//   unbounded copies                        plain forward DP, not this
//   all reachable sums                      iterate _Find_next over dp
//   count of DISTINCT reachable sums        dp.count()
//   coins: fewest to make S                 BFS/DP, not this
//   "can we split into k equal groups"      bitmask DP over items, n <= 20 --
//                                           DP/bitmask_tsp.cpp [1]
//   subset with a given XOR                 xor basis, not subset sum --
//                                           DP/sub-problems xorbasis.cpp
// ============================================================================
const int MAXB = 100005;

// dp[s] == 1  <=>  some subset of w sums to exactly s
bitset<MAXB> reach(const vector<int> &w) {
    bitset<MAXB> dp;
    dp[0] = 1;
    for (int x : w) dp |= dp << x;
    return dp;
}
// k copies of value v, expressed as O(log k) independent items
void push_bounded(vector<int> &items, int v, int k) {
    for (int b = 1; k > 0; b <<= 1) {
        int t = min(b, k);
        items.push_back(t * v);
        k -= t;
    }
}
// which items make S. Returns their indices, or empty if unreachable.
// Keeps one snapshot per item: O(n * MAXB / 8) bytes -- see the pitfalls.
vector<int> recover(const vector<int> &w, int S) {
    int n = w.size();
    vector<bitset<MAXB>> snap(n + 1);
    snap[0][0] = 1;
    for (int i = 0; i < n; i++) snap[i + 1] = snap[i] | (snap[i] << w[i]);
    if (!snap[n][S]) return {};
    vector<int> take;
    int cur = S;
    for (int i = n - 1; i >= 0; i--) {
        if (snap[i][cur]) continue;              // item i was not needed
        take.push_back(i), cur -= w[i];          // it was: undo it
    }
    reverse(take.begin(), take.end());
    return take;
}
