// ============================================================================
// RETROGRADE ANALYSIS -- win/lose/DRAW when the game graph has CYCLES
// ----------------------------------------------------------------------------
// Memoised win/lose DFS assumes the state graph is a DAG. The moment positions
// can repeat, it either infinite-loops or -- worse -- treats an unresolved
// state as a Loss and returns a plausible wrong answer.
//
// The fix is a BFS backwards from the terminal positions, carrying an
// OUT-DEGREE COUNTER per state:
//
//     a position is a WIN   if ANY move leads to a Lose      (1 predecessor
//                                                             resolution)
//     a position is a LOSE  if EVERY move leads to a Win      (decrement the
//                                                             counter to 0)
//     a position is a DRAW  if it is never labelled at all
//
// That last line is the whole point. Draw is not computed; it is what remains.
//
// ############################################################################
// #  YOU MUST WALK THE REVERSE EDGES, AND THE DEGREE IS OF THE FORWARD GRAPH
// #
// #  The BFS propagates from a resolved state to its PREDECESSORS, so you
// #  need rev[]. But `deg[v]` counts v's FORWARD moves. Building deg from the
// #  reverse adjacency is the classic mix-up: it compiles, it runs, and every
// #  "Lose" is wrong.
// ############################################################################
//
// ############################################################################
// #  A STATE WITH NO MOVES IS A LOSS UNDER NORMAL PLAY -- SEED IT
// #
// #  deg[v] == 0 means the player to move cannot move, so under NORMAL play
// #  they LOSE. Those are the BFS sources. Under MISERE play they WIN, and
// #  the only change is the seed label -- everything else is identical.
// #  Forgetting to seed leaves the entire graph a draw.
// ############################################################################
//
// PITFALLS:
//   Once a state is labelled it is never relabelled. Guard with `if (res[u])
//     continue;` on pop, and check before pushing.
//   The counter must be decremented exactly once per (predecessor, edge). With
//     multi-edges, either dedupe or count them consistently in deg[].
//   States are (position, whose turn) ONLY if the two players have different
//     move sets. For an impartial game the position alone is the state.
//   If the game is impartial, acyclic, and you need to COMBINE independent
//     components, this is the wrong tool -- you need Grundy values, not
//     win/lose. game/grundy.cpp.
//   Scoring games are neither: use minimax DP.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   win/lose/draw on a cyclic move graph            this
//   "the game may go on forever -- who wins"        this; Draw = unlabelled
//   two tokens on a graph, players move one          state = (u, v, turn)
//   pursuit / cops and robbers, small graph          state = (cop, robber,
//                                                    turn), this
//   a partizan game (different move sets)            state = (pos, turn), this
//   normal play, "cannot move loses"                 seed deg==0 as LOSE
//   misere play, "cannot move wins"                  seed deg==0 as WIN,
//                                                    nothing else changes
//   the fastest win / slowest loss                   carry a distance: WIN
//                                                    takes min over losing
//                                                    successors + 1, LOSE
//                                                    takes max
//   impartial, ACYCLIC, several independent piles    NOT this -- Grundy,
//                                                    game/grundy.cpp
//
// NOT THIS:
//   acyclic and impartial and you need to xor components -> Grundy.
//   players accumulate a score                            -> minimax DP.
//   the state space is too large to enumerate             -> find the pattern
//     instead; retrograde needs every state materialised.
// ============================================================================
const int WIN = 1, LOSE = 2, DRAW = 0;

int n;                          // number of states
vector<int> fwd[1000005];       // fwd[u] = states reachable in one move
vector<int> rev_[1000005];      // rev_[v] = states that can move TO v
int deg[1000005];               // OUT-degree in the FORWARD graph
int res[1000005];               // WIN / LOSE / DRAW
int dist_[1000005];             // moves to the resolution, optional

void build_rev() {
    for (int u = 0; u < n; u++) {
        deg[u] = fwd[u].size();
        for (int v : fwd[u]) rev_[v].push_back(u);
    }
}

// normal_play = true : a player with no moves LOSES (the usual rule)
void solve(bool normal_play = true) {
    for (int u = 0; u < n; u++) res[u] = DRAW, dist_[u] = -1;
    queue<int> q;
    for (int u = 0; u < n; u++)
        if (deg[u] == 0) {                        // terminal: seed it
            res[u] = normal_play ? LOSE : WIN;
            dist_[u] = 0;
            q.push(u);
        }
    while (!q.empty()) {
        int v = q.front(); q.pop();
        for (int u : rev_[v]) {
            if (res[u] != DRAW) continue;         // already decided
            if (res[v] == LOSE) {                 // one losing move is enough
                res[u] = WIN;
                dist_[u] = dist_[v] + 1;
                q.push(u);
            } else if (--deg[u] == 0) {           // every move leads to a WIN
                res[u] = LOSE;
                dist_[u] = dist_[v] + 1;
                q.push(u);
            }
        }
    }
    // anything still DRAW is a genuine draw: the mover can avoid losing forever
}
