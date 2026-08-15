// ============================================================================
// HUNGARIAN -- minimum-cost perfect assignment, O(n^2 m)
// ----------------------------------------------------------------------------
// n workers, m jobs (n <= m), cost a[i][j]. Assign every worker to a distinct
// job minimising the total. Unlike Kuhn this is about COST, not just whether a
// matching exists.
//
// ############################################################################
// #  1-INDEXED, AND ROW 0 / COLUMN 0 ARE WORKING SPACE
// #
// #  a must be (n+1) x (m+1) with real data in [1..n][1..m]. Column 0 is the
// #  sentinel the augmentation starts from, and v[0] accumulates the answer.
// #  Passing a 0-indexed matrix does not crash -- it silently reads zeros as
// #  costs and returns a wrong, plausible number.
// #
// #  n <= m IS REQUIRED. With more workers than jobs, transpose first.
// ############################################################################
//
// ############################################################################
// #  THE COST IS -v[0], NOT A SUM YOU ACCUMULATE
// #
// #  The potentials u/v encode the answer; v[0] ends at the negated optimum.
// #  Summing a[i][assign[i]] yourself also works and is a good self-check --
// #  if the two disagree, the input was not 1-indexed.
// ############################################################################
//
// PITFALLS:
//   MAXIMISE instead: negate every cost, then negate the answer. Do not try to
//     flip the comparisons.
//   Costs may be negative; that is fine. INF must exceed n * max|cost|.
//   Non-square is fine (n <= m); every worker gets a job, some jobs idle.
//   If some pairs are FORBIDDEN, set them to a large finite value, not INF --
//     INF arithmetic overflows inside the potential updates.
//   O(n^2 m) means n = 500 is comfortable, n = 2000 is not.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   min cost to assign n tasks to n workers      the literal reading
//   max total profit assignment                  negate the matrix
//   min total distance pairing two point sets    cost = distance
//   "match every A to a distinct B, minimise      same
//    the worst/total cost"
//   minimise the MAXIMUM cost (bottleneck)       NOT this -- binary search the
//                                                answer + bipartite matching
//                                                (graph/matching/kuhn.cpp)
//   assignment with capacities                   MCMF, graph/flows/MCMF.cpp
//   just "does a perfect matching exist"         Kuhn, much faster
//   costs on a general (non-bipartite) graph     blossom with weights -- no
//
// NOT THIS when the sizes are large but the structure is special (a grid, a
//   tree, convex costs) -- those usually have a greedy or flow formulation
//   that beats O(n^3).
// ============================================================================
const ll HINF = (ll)4e18 / 4;

// a is (n+1) x (m+1), 1-indexed, n <= m.
// assign[j] = the worker matched to job j (0 if none). Returns the min cost.
ll hungarian(const vector<vector<ll>> &a, int n, int m, vector<int> &assign) {
    vector<ll> u(n + 1, 0), v(m + 1, 0);
    vector<int> p(m + 1, 0), way(m + 1, 0);
    for (int i = 1; i <= n; i++) {
        p[0] = i;
        int j0 = 0;
        vector<ll> minv(m + 1, HINF);
        vector<char> used(m + 1, false);
        do {
            used[j0] = true;
            int i0 = p[j0], j1 = -1;
            ll delta = HINF;
            for (int j = 1; j <= m; j++)
                if (!used[j]) {
                    ll cur = a[i0][j] - u[i0] - v[j];
                    if (cur < minv[j]) minv[j] = cur, way[j] = j0;
                    if (minv[j] < delta) delta = minv[j], j1 = j;
                }
            for (int j = 0; j <= m; j++) {
                if (used[j]) u[p[j]] += delta, v[j] -= delta;
                else minv[j] -= delta;
            }
            j0 = j1;
        } while (p[j0] != 0);
        do {                                  // walk the augmenting path back
            int j1 = way[j0];
            p[j0] = p[j1], j0 = j1;
        } while (j0);
    }
    assign.assign(m + 1, 0);
    for (int j = 1; j <= m; j++) assign[j] = p[j];
    return -v[0];                             // see the box
}
