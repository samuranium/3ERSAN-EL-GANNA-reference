// ============================================================================
// BROKEN PROFILE DP -- grid DP cell by cell, O(n * m * 2^m)
// ----------------------------------------------------------------------------
// A row-by-row grid DP needs the whole previous row as state: 2^m per row, and
// the transition between two full rows is another 2^m -- O(n * 4^m). Broken
// profile moves ONE CELL at a time instead, so the state is a single mask that
// straddles the boundary between the finished part and the rest, and the
// transition is O(1). O(n * m * 2^m) total.
//
// ############################################################################
// #  WHAT THE MASK MEANS -- get this wrong and nothing else matters
// #
// #  Bit 0 is the CURRENT cell. Bit k is the cell k steps later in row-major
// #  order. A set bit means "already covered by a piece placed earlier".
// #
// #  Moving to the next cell SHIFTS the mask right by one. A vertical piece
// #  reaches m cells ahead, so it sets bit m-1 AFTER the shift. A horizontal
// #  piece reaches one cell ahead, so it sets bit 0 after the shift.
// #
// #  That is the whole algorithm. Everything else is bounds checks.
// ############################################################################
//
// ############################################################################
// #  MAKE m THE SMALLER DIMENSION
// #
// #  Cost is 2^m, so a 2 x 20 grid must be processed as 20 x 2, not 2 x 20.
// #  swap(n, m) first -- the answer is identical and the runtime is not.
// #  m <= 20 is the practical ceiling; m <= 12 is comfortable.
// ############################################################################
//
// PITFALLS:
//   A vertical piece needs i + 1 < n; a horizontal needs j + 1 < m AND the
//     next cell free (bit 1 of the pre-shift mask clear).
//   The final answer is dp[0]: every cell covered, nothing dangling.
//   Blocked cells are not an exception -- treat a blocked cell as "already
//     covered" and just shift.
//   Counts explode; take a modulus unless n*m is tiny.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   tile a grid with 1x2 dominoes           count_tilings()
//   ... with some cells blocked             blocked[i][j] -> treat as covered
//   ... count tilings mod p                 same, add the mod
//   maximum independent set on a grid       mask = "did I take the cell m
//     (no two adjacent chosen)              back", value DP instead of count
//   place k non-attacking pieces            add the count to the state:
//                                           dp[cell][mask][k]
//   colour a grid, adjacent differ          mask holds the colours of the last
//                                           m cells (base-c, not binary)
//   count Hamiltonian circuits on a grid    PLUG DP -- the mask holds bracket
//     / connected shapes                    plugs, not booleans, and needs a
//                                           canonical form per state. Much
//                                           heavier; this file is the boolean
//                                           special case
//   longest path in a grid                  plug DP again, not this
//
// NOT THIS: if the grid is huge in BOTH dimensions, there is no profile DP --
//   look for a formula (domino tilings have one), a matching formulation
//   (grid tiling = perfect matching on a bipartite graph, graph/matching/),
//   or a transfer matrix raised to a power.
// ============================================================================

// number of ways to tile an n x m grid with 1x2 dominoes
ll count_tilings(int n, int m) {
    if (m > n) swap(n, m);                       // 2^m cost -- see the box
    vector<ll> dp(1 << m, 0), nd;
    dp[0] = 1;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++) {
            nd.assign(1 << m, 0);
            for (int mask = 0; mask < (1 << m); mask++) {
                if (!dp[mask]) continue;
                if (mask & 1) {                  // already covered: just advance
                    nd[mask >> 1] += dp[mask];
                } else {
                    if (i + 1 < n)               // vertical: reaches m ahead
                        nd[(mask >> 1) | (1 << (m - 1))] += dp[mask];
                    if (j + 1 < m && !(mask & 2))// horizontal: reaches 1 ahead
                        nd[(mask >> 1) | 1] += dp[mask];
                }
            }
            dp.swap(nd);
        }
    return dp[0];                                // nothing left dangling
}
// same, with blocked cells. blocked[i][j] = 1 means the cell cannot be used.
ll count_tilings_blocked(int n, int m, const vector<vector<int>> &blocked) {
    vector<ll> dp(1 << m, 0), nd;
    dp[0] = 1;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++) {
            nd.assign(1 << m, 0);
            for (int mask = 0; mask < (1 << m); mask++) {
                if (!dp[mask]) continue;
                if (blocked[i][j]) {             // treat as already covered
                    if (!(mask & 1)) nd[mask >> 1] += dp[mask];
                    continue;                    // a piece must not overlap it
                }
                if (mask & 1) {
                    nd[mask >> 1] += dp[mask];
                } else {
                    if (i + 1 < n && !blocked[i + 1][j])
                        nd[(mask >> 1) | (1 << (m - 1))] += dp[mask];
                    if (j + 1 < m && !(mask & 2) && !blocked[i][j + 1])
                        nd[(mask >> 1) | 1] += dp[mask];
                }
            }
            dp.swap(nd);
        }
    return dp[0];
}
