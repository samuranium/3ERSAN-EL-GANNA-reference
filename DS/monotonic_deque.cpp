// ============================================================================
// MONOTONIC DEQUE -- sliding window minimum / maximum, O(n)
// ----------------------------------------------------------------------------
// The deque holds INDICES whose values are monotone. Anything smaller than the
// incoming value can never be the maximum again, so it is dropped from the
// back; anything that has slid out of the window is dropped from the front.
// Each index enters and leaves once -> O(n) for ALL windows, not per window.
//
// ############################################################################
// #  ORDER OF THE THREE STEPS
// #      1. pop_back  while the back is worse than a[i]
// #      2. push_back i
// #      3. pop_front if the front has left the window
// #  Doing 3 before 2 can pop the element you are about to need when k == 1.
// #  Store INDICES, never values -- you cannot test "has it expired" otherwise.
// ############################################################################
//
// 0-INDEXED. out[i] is the answer for the window a[i .. i+k-1], so out is
// filled for i in [0, n-k].
//
// SUBPROBLEMS:
//   sliding window max / min           window_max, flip <= to >= for min
//   longest subarray with max-min <= x two deques (one max, one min) plus two
//                                      pointers; shrink while the spread is too big
//   min over every window of every size that is a monotonic STACK job, not this
//   DP with a window transition        dp[i] = min(dp[j] : i-k <= j < i) + c[i];
//                                      push dp values instead of a[] -- this is
//                                      the "deque optimisation" of a 1D DP
//   first negative / first > x in each  same sweep, keep the deque on the
//     window                            predicate instead of the value
//   max of a[i..j] with j-i fixed       exactly window_max
//
// RELATED: constant-factor cousin of a sparse table. If the window SIZE varies
//   per query and the array is static, use DS/sparse/Sparse Table.cpp instead --
//   O(1) per arbitrary range. This file wins only when the window slides.
// ============================================================================
const int N = 200'123;
ll a[N], out[N];

// out[i] = max of a[i .. i+k-1], for i in [0, n-k]
void window_max(int n, int k) {
    deque<int> dq;
    for (int i = 0; i < n; i++) {
        while (dq.size() && a[dq.back()] <= a[i]) dq.pop_back();   // >= for MIN
        dq.push_back(i);
        if (dq.front() <= i - k) dq.pop_front();
        if (i >= k - 1) out[i - k + 1] = a[dq.front()];
    }
}
// longest subarray whose (max - min) <= x. Returns the length.
int longest_spread(int n, ll x) {
    deque<int> mx, mn;
    int best = 0;
    for (int i = 0, l = 0; i < n; i++) {
        while (mx.size() && a[mx.back()] <= a[i]) mx.pop_back();
        mx.push_back(i);
        while (mn.size() && a[mn.back()] >= a[i]) mn.pop_back();
        mn.push_back(i);
        while (a[mx.front()] - a[mn.front()] > x) {     // shrink from the left
            if (mx.front() == l) mx.pop_front();
            if (mn.front() == l) mn.pop_front();
            l++;
        }
        best = max(best, i - l + 1);
    }
    return best;
}
