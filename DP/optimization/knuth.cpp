// ============================================================================
// KNUTH (Knuth-Yao) OPTIMIZATION -- interval DP, O(n^3) -> O(n^2)
// ----------------------------------------------------------------------------
// SHAPE IT SOLVES -- must match exactly:
//     dp[i][j] = min over k in [i, j-1] of ( dp[i][k] + dp[k+1][j] ) + C(i, j)
//   Split into two ADJACENT intervals, plus a cost depending on (i, j) ONLY.
//   Cost depends on k? Wrong shape -- see NOT THIS.
//
// WHEN IT IS LEGAL -- C must satisfy BOTH, for all a <= b <= c <= d:
//   1. quadrangle inequality   C(a,c) + C(b,d) <= C(a,d) + C(b,c)
//   2. monotone on intervals   C(b,c) <= C(a,d)            (inner <= outer)
//   These give  opt[i][j-1] <= opt[i][j] <= opt[i+1][j],  and THAT is what
//   collapses the inner loop -- to O(n^2) total, amortised, not per state.
//
// THE ONLY CHECK YOU NEED UNDER TIME PRESSURE:
//   Is C(i,j) the sum of a[i..j] with every a[i] >= 0?  -> yes, both hold.
//   Covers the whole "merge two adjacent piles, cost = size of the merged pile"
//   family, and optimal BST (Knuth's original problem). Anything else: brute
//   force the quadrangle inequality on random n <= 6 arrays before trusting it.
//
// NOT THIS -- the three it gets confused with:
//   dp[layer][j] = min_k dp[layer-1][k] + C(k,j)  -> divide & conquer opt,
//                                                    D&C.cpp in this folder
//   cost depends on the split point k itself      -> not Knuth (matrix chain
//                                                    multiplication is this)
//   dp[i] = min_k dp[k] + m[k]*x[i] + c[k]        -> CHT / Li Chao
//
// DON'T BOTHER WHEN n <= ~500: plain O(n^3) is ~1.2e8 and passes, and you skip
//   the risk of a quadrangle inequality you never actually verified.
//
// HOW TO USE:
//   1. fill pre[]  -- prefix sums, pre[i+1] = pre[i] + a[i]
//   2. rewrite C() for the problem
//   3. knuth(n);   answer = dp[0][n-1]
//   Length-increasing order is mandatory: opt[i][j-1] and opt[i+1][j] must
//   already be final when dp[i][j] is computed.
//
// SYMPTOM THAT C IS WRONG: some dp[i][j] stays LLONG_MAX. That means lo > hi,
//   i.e. opt[i][j-1] > opt[i+1][j], i.e. the quadrangle inequality fails.
//
// MEMORY: dp + opt at N = 2005 is 46 MB. Shrink N if anything else is big.
// ============================================================================
const int N = 2005;
long long dp[N][N], pre[N];
int opt[N][N];

// cost of segment [i, j]; must be O(1) -- problem-specific
inline long long C(int i, int j) { return pre[j + 1] - pre[i]; }

void knuth(int n) {
    for (int i = 0; i < n; i++) { dp[i][i] = 0; opt[i][i] = i; }
    for (int len = 2; len <= n; len++)
        for (int i = 0, j = len - 1; j < n; i++, j++) {
            dp[i][j] = LLONG_MAX;
            int lo = max(opt[i][j - 1], i);
            int hi = min(opt[i + 1][j], j - 1);
            for (int k = lo; k <= hi; k++) {
                long long cur = dp[i][k] + dp[k + 1][j] + C(i, j);
                if (cur < dp[i][j]) { dp[i][j] = cur; opt[i][j] = k; }
            }
        }
}