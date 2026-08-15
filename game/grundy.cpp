// ============================================================================
// SPRAGUE-GRUNDY -- impartial games, normal play
// ----------------------------------------------------------------------------
// CONDITIONS. All four must hold or none of this applies:
//   two players alternate, both play optimally
//   both have the SAME moves available (impartial -- not chess, not any game
//     where the players own different pieces)
//   finite, no draws, no repetition
//   NORMAL PLAY: the player unable to move LOSES. (Misere is different; see
//     the end.)
//
// ############################################################################
// #  THE WHOLE THEORY IN THREE LINES
// #
// #    G(position) = mex{ G(p) : p reachable in one move }
// #    G == 0  <=>  the player TO MOVE loses
// #    G(A + B + ...) = G(A) xor G(B) xor ...      <- independent subgames
// #
// #  mex = smallest non-negative integer NOT in the set. mex{} = 0, which is
// #  why a position with no moves is a loss.
// #
// #  The xor is the entire point. It is why Nim is the universal impartial
// #  game and why "several independent piles/heaps/components" is always the
// #  same problem regardless of the theme.
// ############################################################################
//
// NIM: piles of sizes a1..ak, take any positive number from one pile.
//   G(pile of n) = n, so the whole game is xor of the sizes.
//   First player wins iff the xor is nonzero. The winning move is: find a pile
//   with (a_i xor total) < a_i and reduce it to that.
//
// ############################################################################
// #   RECOGNISING IT
// #
// #   "several independent piles / stacks / heaps / components, a move
// #    touches exactly one of them"  ->  Sprague-Grundy, xor the parts.
// #
// #   If a move can touch TWO components, the xor rule does NOT apply and you
// #   are in a different (usually much harder) game.
// ############################################################################
//
// PITFALLS:
//   Grundy values are NOT the number of moves, and NOT 0/1. Computing "win or
//     lose" per component and xoring the booleans is wrong -- you need the
//     full value whenever components are combined.
//   For a single game with no sum, win/lose (G == 0 or not) is enough and you
//     can skip mex entirely.
//   The table below is O(maxN * moves). If the state space is large, look for
//     a pattern in the first 30 values -- impartial games are periodic
//     astonishingly often, and proving the period is usually easier than
//     computing it.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS / KNOWN GAMES
// ----------------------------------------------------------------------------
//   Nim                                 xor of pile sizes
//   subtraction game (take s in S)      grundy_subtraction() below
//   take 1..k from a pile               G(n) = n mod (k+1)
//   Misere Nim                          same as Nim UNLESS every pile is 1;
//                                       then the parity of the pile count
//                                       flips the answer
//   Staircase Nim                       only the ODD-indexed steps count; xor
//                                       those, ignore the rest
//   split a pile into unequal parts     G by mex over all splits
//   Green Hackenbush / edge deletion    grundy_graph() below -- Colon and
//     on a graph rooted at a vertex     Fusion principles
//   coin turning games                  Grundy of a single coin position,
//                                       xored over the heads
//   Wythoff (two piles, take from one   losing positions are the Beatty pairs
//     or equally from both)             (floor(k*phi), floor(k*phi^2))
//   partizan games (players differ)     NOT this -- surreal numbers
// ============================================================================
const int GN = 300005;

// mex of a small set of values
int mex(vector<int> v) {
    sort(v.begin(), v.end());
    int m = 0;
    for (int x : v) {
        if (x > m) break;
        if (x == m) m++;
    }
    return m;
}
// G[n] for the subtraction game: from a pile of n you may remove any s in S.
int G[GN];
void grundy_subtraction(int maxn, const vector<int> &S) {
    for (int n = 0; n <= maxn; n++) {
        vector<int> reach;
        for (int s : S)
            if (n - s >= 0) reach.push_back(G[n - s]);
        G[n] = mex(reach);
    }
}
// whole game = several piles: xor the values, nonzero means first player wins
bool first_player_wins(const vector<int> &piles) {
    int x = 0;
    for (int p : piles) x ^= G[p];
    return x != 0;
}

// ---------------------------------------------------------------------------
// GRUNDY ON A GRAPH -- players delete edges; any component disconnected from
// the root is discarded; a player who cannot move loses.
//
// Two principles do the work:
//   COLON   the Grundy value of a tree is the xor of (1 + G(child subtree))
//           over its children -- a bridge contributes one extra move.
//   FUSION  two vertices on a common cycle may be contracted without changing
//           the value. So contract every 2-edge-connected component and the
//           graph becomes a tree with self-loops; each self-loop is worth 1.
//
// Taken from the other team's reference (SPOJ 1477, IPSC 2003 G).
// ---------------------------------------------------------------------------
vector<int> gg[GN];
int glow[GN], gdis[GN], gT, gn;

int grundy_graph(int u, int pre = 0) {
    gdis[u] = glow[u] = ++gT;
    int ans = 0;
    for (int v : gg[u]) {
        if (v == pre) { pre += 2 * gn; continue; }     // skip ONE parent edge
        if (gdis[v] == 0) {
            int res = grundy_graph(v, u);
            glow[u] = min(glow[u], glow[v]);
            if (glow[v] > gdis[u]) ans ^= (1 + res) ^ 1;   // bridge: Colon
            else ans ^= res;                              // inside a cycle
        } else glow[u] = min(glow[u], gdis[v]);
    }
    if (pre > gn) pre -= 2 * gn;
    for (int v : gg[u])
        if (v != pre && gdis[u] <= gdis[v]) ans ^= 1;      // self / back edges
    return ans;
}
