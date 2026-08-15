// ============================================================================
// TREE KNAPSACK -- "choose exactly k nodes in the tree", O(n*k) not O(n*k^2)
// ----------------------------------------------------------------------------
// dp[v][j] = best value using exactly j chosen nodes inside v's subtree.
// Merge children one at a time, like a knapsack where each child is an item
// whose "weights" are 0..size(child).
//
// ############################################################################
// #  THE TWO LOOP CAPS ARE THE WHOLE ALGORITHM
// #
// #      for (int a = min(done, k); a >= 0; a--)          // NOT a = k
// #          for (int b = min(sz[c], k - a); b >= 0; b--) // NOT b = k
// #
// #  `done` is the number of nodes merged into v SO FAR; sz[c] is the child's
// #  subtree size. With both caps the total work is O(n*k) (O(n^2) when
// #  k = n). Looping to k unconditionally is O(n*k^2) and produces the
// #  IDENTICAL answer -- it just TLEs. This is the single most common way to
// #  write a correct tree knapsack that does not pass.
// #
// #  Why O(n^2): the pair (a,b) is charged to one pair of NODES, one from the
// #  already-merged part and one from the child. Every pair of nodes is
// #  charged exactly once -- at their LCA. n^2/2 pairs, done.
// ############################################################################
//
// ############################################################################
// #  MERGE INTO A TEMPORARY, NEVER IN PLACE
// #
// #  dp[v] is being read (index a) and written (index a+b) in the same pass.
// #  b = 0 writes onto the cell you are about to read. Build `ndp` fresh,
// #  then swap. Descending loops are NOT enough to save you here, because
// #  the child contributes at every b, not just b = 1.
// ############################################################################
//
// PITFALLS:
//   Identity: for MAX use -INF everywhere and dp[v][0 or 1] = base, so that
//     unreachable counts can never be selected. Initialising to 0 silently
//     legalises "j nodes chosen" when it is impossible.
//   ndp must be sized min(done + sz[c], k) + 1 and filled with -INF each time.
//   Recursion depth: a path graph with n = 2e5 overflows the stack. Iterative
//     DFS, or raise the stack.
//   sz[v] must be accumulated as you merge, not read from a separate pass, or
//     the cap is wrong for the first children.
//   "exactly k" vs "at most k": for at-most, answer = max over j <= k, or add
//     a free 0-value transition.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   choose exactly k nodes, maximise value            solve() below
//   choose exactly k EDGES                            index by edges; a subtree
//                                                     with j nodes has j-1
//                                                     internal edges
//   pick k nodes, they must be CONNECTED              force v chosen, and only
//                                                     merge children that are
//                                                     themselves connected to v
//   cut the tree into exactly k components            dp[v][j][0/1] = is the
//                                                     edge to the parent cut
//   minimum cost to keep exactly k leaves             same shape, cost on edges
//   count subtrees of size exactly k                  same loop, += instead of
//                                                     max
//   maximum weight independent set of size k          add a 0/1 "is v taken"
//                                                     dimension
//   knapsack with a numeric CAPACITY, not a count     the Euler-tour form
//                                                     below -- O(n*W)
//   "each node needs its parent taken first"          that is exactly the
//                                                     Euler-tour form
//
// NOT THIS:
//   no count/capacity dimension at all -> plain tree DP, one value per node.
//   the capacity is huge AND the count matters -> the product is the cost;
//     neither form saves you.
// ============================================================================
const ll NEG = -1e18 / 4;

int n, K;
vector<int> g[200005];
ll  val[200005];             // value of taking node v
int sz[200005];
vector<ll> dp[200005];       // dp[v][j], j = 0..min(sz[v], K)

void dfs(int v, int p) {
    sz[v] = 1;
    dp[v].assign(min(1, K) + 1, NEG);
    dp[v][0] = 0;                                  // take nothing from v
    if (K >= 1) dp[v][1] = val[v];                 // take v itself
    int done = 1;                                  // nodes merged so far
    for (int c : g[v]) {
        if (c == p) continue;
        dfs(c, v);
        int lim = min(done + sz[c], K);
        vector<ll> ndp(lim + 1, NEG);
        for (int a = min(done, K); a >= 0; a--) {
            if (dp[v][a] == NEG) continue;
            int bmax = min(sz[c], K - a);
            for (int b = bmax; b >= 0; b--) {
                if (dp[c][b] == NEG) continue;
                ndp[a + b] = max(ndp[a + b], dp[v][a] + dp[c][b]);
            }
        }
        dp[v].swap(ndp);
        done += sz[c];
        sz[v] += sz[c];
    }
}

ll solve(int root) {                               // exactly K nodes
    dfs(root, 0);
    return dp[root].size() > (size_t)K ? dp[root][K] : NEG;
}

////////////////////////////////////////////////////////////////////////////////
// EULER-TOUR FORM -- capacity W instead of a node count, O(n*W)
//
// Use when each node has a WEIGHT and the budget is a number, not a count, and
// a node may be taken only if its parent is. Flatten to DFS order; at position
// i you either TAKE node ord[i] and move to i+1, or SKIP ITS WHOLE SUBTREE and
// jump to tout[ord[i]]. No child merging at all -- it is a 1D knapsack with
// jumps, and it is far shorter than the version above.
//
// f[i][w] = best value from positions i.. with budget w.
////////////////////////////////////////////////////////////////////////////////
int ord[200005], pos_[200005], tout_[200005], timer_;
int wt[200005];

void flatten(int v, int p) {
    pos_[v] = timer_;
    ord[timer_++] = v;
    for (int c : g[v]) if (c != p) flatten(c, v);
    tout_[v] = timer_;                             // one past the subtree
}

ll knap_tour(int root, int W) {
    timer_ = 0;
    flatten(root, 0);
    // f[i][w]; i from n down to 0
    vector<vector<ll>> f(n + 1, vector<ll>(W + 1, 0));
    for (int i = n - 1; i >= 0; i--) {
        int v = ord[i];
        for (int w = 0; w <= W; w++) {
            ll best = f[tout_[v]][w];              // skip v and all below it
            if (wt[v] <= w) best = max(best, val[v] + f[i + 1][w - wt[v]]);
            f[i][w] = best;
        }
    }
    return f[0][W];
}
