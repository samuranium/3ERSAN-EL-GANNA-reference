// ============================================================================
// LIS -- longest increasing subsequence, O(n log n), with reconstruction
// ----------------------------------------------------------------------------
// tail[k] = the SMALLEST possible tail value of an increasing subsequence of
// length k+1 seen so far. It is sorted by construction, so each element does
// one binary search. tail is NOT the answer subsequence -- it is a set of best
// candidates, and printing it is the classic wrong output.
//
// ############################################################################
// #  STRICT vs NON-DECREASING IS ONE FUNCTION CALL
// #
// #    strictly increasing  (a < b)      lower_bound
// #    non-decreasing       (a <= b)     upper_bound
// #
// #  Nothing else changes. Getting it backwards is off by exactly the number
// #  of duplicate runs, so it passes samples with distinct values.
// #
// #  DECREASING: negate the array, or reverse it -- do not flip comparators,
// #  because lower/upper_bound then need a custom comparator too and the two
// #  changes interact.
// ############################################################################
//
// RECONSTRUCTION: par[i] is the index that preceded i in the best subsequence
//   ending at i. It is read from tailIdx[p-1] BEFORE tailIdx[p] is overwritten
//   -- position p-1 is untouched by the write at p, which is why this is safe.
//
// PITFALLS:
//   Empty input: guard before touching tailIdx.back().
//   The returned vector holds INDICES, not values. a[i] for the values.
//   Ties in reconstruction pick one valid answer, not a specific one.
//   O(n log n) is on the LENGTH; if you need the count of distinct LIS or
//     weights, that is a Fenwick DP, not this.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   LIS length                          lis().size()
//   the subsequence itself              lis() returns indices
//   longest NON-DECREASING              upper_bound instead
//   longest DECREASING / non-increasing negate the array, then the above
//   number of DIFFERENT LIS             Fenwick over values keeping (len, cnt)
//   maximum-SUM increasing subsequence  Fenwick over values with max-sum, not
//                                       this greedy
//   LIS of a[l..r] for many queries     offline + merge sort tree, or a
//                                       different formulation -- greedy LIS
//                                       does not restrict to ranges
//   2D: pairs (x,y), both increasing    sort by x (ties by y DESCENDING to
//                                       forbid equal x), then LIS on y
//   3D and up                           CDQ divide and conquer, or a BIT of
//                                       BITs -- DS/BIT/fenwick_2d.cpp
//   LCS of two PERMUTATIONS             map one to positions in the other,
//                                       then LIS. O(n log n) instead of O(n^2)
//   minimum number of increasing        = longest NON-INCREASING subsequence
//     subsequences covering the array   (Dilworth)
//   minimum chains covering a poset     Dilworth again -- longest antichain
//   patience sorting / card piles       tail[] IS the pile tops
//   "minimum removals to make it        n - LIS
//     increasing"
// ============================================================================

// returns the INDICES of one longest strictly increasing subsequence
vector<int> lis(const vector<int> &a) {
    int n = a.size();
    if (!n) return {};
    vector<int> tail, tailIdx, par(n, -1);
    for (int i = 0; i < n; i++) {
        int p = lower_bound(tail.begin(), tail.end(), a[i]) - tail.begin();
        //                  ^^^^^^^^^^^ upper_bound for NON-DECREASING
        par[i] = p ? tailIdx[p - 1] : -1;        // read p-1 before writing p
        if (p == (int)tail.size()) tail.push_back(a[i]), tailIdx.push_back(i);
        else tail[p] = a[i], tailIdx[p] = i;
    }
    vector<int> res;
    for (int i = tailIdx.back(); i != -1; i = par[i]) res.push_back(i);
    reverse(res.begin(), res.end());
    return res;
}
int lis_length(const vector<int> &a) {           // length only, no bookkeeping
    vector<int> tail;
    for (int x : a) {
        int p = lower_bound(tail.begin(), tail.end(), x) - tail.begin();
        if (p == (int)tail.size()) tail.push_back(x);
        else tail[p] = x;
    }
    return tail.size();
}
