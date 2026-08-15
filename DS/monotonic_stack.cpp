// ============================================================================
// MONOTONIC STACK -- nearest smaller/greater, O(n)
// ----------------------------------------------------------------------------
// One sweep, each index pushed and popped once. Everything below is the same
// two loops with a different comparison.
//
// ############################################################################
// #  THE STRICT / NON-STRICT ASYMMETRY IS NOT A TYPO
// #
// #      left  pops on >=   -> L[i] = nearest STRICTLY smaller on the left
// #      right pops on >    -> R[i] = nearest smaller-OR-EQUAL on the right
// #
// #  With equal values, symmetric rules make a subarray get counted once per
// #  tied minimum. Breaking the tie on exactly one side counts it ONCE.
// #  Largest-rectangle survives symmetric rules; sum-over-subarrays does not.
// #  Use the asymmetric pair always and you never have to think about it.
// ############################################################################
//
// 0-INDEXED here. L[i] = -1 and R[i] = n mean "nothing smaller on that side",
// which makes the width formulas below work with no special cases.
//
// SUBPROBLEMS:
//   nearest smaller left / right       L[], R[]
//   nearest GREATER instead            flip both comparisons (<= and <)
//   largest rectangle in a histogram   a[i] * (R[i] - L[i] - 1)
//   # subarrays where i is the minimum (i - L[i]) * (R[i] - i)
//   sum over subarrays of the minimum  sum of a[i] * that count
//   sum over subarrays of (max - min)  run it twice, subtract
//   maximal rectangle in a 0/1 matrix  histogram per row, this per row
//   next greater element, circular     run over the array twice (i % n)
//   previous smaller for a stack DP    e.g. largest square, skyline problems
//
// NOT THIS: if the array CHANGES between queries, a monotonic stack is offline
//   only -- use a segment tree with a descend (DS/segment tree/lazy.cpp [1]).
// ============================================================================
const int N = 200'123;
ll a[N];
int L[N], R[N];

// L[i]: nearest index on the left with a STRICTLY smaller value, else -1
// R[i]: nearest index on the right with a smaller-OR-EQUAL value, else n
void nearest_smaller(int n) {
    stack<int> st;
    for (int i = 0; i < n; i++) {
        while (st.size() && a[st.top()] >= a[i]) st.pop();
        L[i] = st.size() ? st.top() : -1;
        st.push(i);
    }
    while (st.size()) st.pop();
    for (int i = n - 1; i >= 0; i--) {
        while (st.size() && a[st.top()] > a[i]) st.pop();
        R[i] = st.size() ? st.top() : n;
        st.push(i);
    }
}
// largest rectangle under a histogram
ll largest_rect(int n) {
    nearest_smaller(n);
    ll best = 0;
    for (int i = 0; i < n; i++)
        best = max(best, a[i] * (R[i] - L[i] - 1));
    return best;
}
// sum, over every subarray, of that subarray's minimum
ll sum_of_minimums(int n) {
    nearest_smaller(n);
    ll s = 0;
    for (int i = 0; i < n; i++)
        s += a[i] * (i - L[i]) * (R[i] - i);
    return s;
}
