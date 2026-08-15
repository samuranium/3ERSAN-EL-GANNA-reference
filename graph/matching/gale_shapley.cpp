// ============================================================================
// GALE-SHAPLEY -- STABLE matching, O(n^2)
// ----------------------------------------------------------------------------
// n proposers and n acceptors, each with a full ranked preference list. A
// matching is STABLE if no pair (p, a) both prefer each other to their current
// partners -- nobody would rather swap. One always exists, and this finds it.
//
//     each free proposer proposes to the best acceptor who has not yet
//     rejected them; the acceptor keeps whichever of (current, new) they
//     prefer and rejects the other.
//
// ############################################################################
// #  THE PROPOSING SIDE GETS ITS OPTIMUM, THE OTHER GETS ITS WORST
// #
// #  Among ALL stable matchings, every proposer receives the BEST partner
// #  they could have in any of them, and every acceptor receives the WORST.
// #  This is not a tie-break detail -- it is the answer to a different
// #  question depending on who proposes.
// #
// #  If the statement says "the men's optimal assignment", men propose. If it
// #  says "the women's", swap the roles. Running it the wrong way round gives
// #  a stable matching, so no test catches it except the one that checks the
// #  specific answer.
// ############################################################################
//
// ############################################################################
// #  THE ACCEPTOR NEEDS A RANK LOOKUP TABLE, NOT A LIST SCAN
// #
// #      rank[a][p] = position of proposer p in acceptor a's list
// #
// #  Comparing two proposers by scanning the preference list is O(n) per
// #  comparison and makes the whole thing O(n^3). Invert each acceptor's list
// #  once, up front. This is the difference between AC and TLE at n = 2000.
// ############################################################################
//
// PITFALLS:
//   `nxt[p]` = how many proposals p has already made. A proposer never
//     proposes twice to the same acceptor; forgetting to advance it loops
//     forever.
//   Preference lists must be COMPLETE permutations for the classic version.
//   Incomplete lists (someone would rather stay single) still work but you
//     must allow "unmatched" as an outcome and check acceptability both ways.
//   Unequal sizes: pad with dummy partners ranked last, or handle free
//     proposers who exhaust their list.
//   The loop is O(n^2) total proposals because each (p, a) pair is proposed
//     at most once.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   stable marriage / stable roommates (two sides)   stable_matching()
//   "no two would rather swap"                        this
//   hospital-residents with quotas                    an acceptor holds q
//                                                     partners in a heap
//                                                     instead of one
//   proposer-optimal vs acceptor-optimal               swap which side proposes
//   "is this given matching stable"                    check all n^2 pairs
//                                                     directly, O(n^2) --
//                                                     is_stable() below
//   the number of stable matchings                     not this; it can be
//                                                     exponential
//   maximise total satisfaction instead of stability   that is an ASSIGNMENT
//                                                     problem -- Hungarian,
//                                                     graph/matching/hungarian.cpp
//   just a maximum matching, no preferences             Kuhn / Hopcroft-Karp
//
// NOT THIS:
//   you want minimum total cost   -> Hungarian. Stability and optimality are
//     different objectives and generally have different answers.
//   one side has no preferences   -> any maximum matching is stable.
//   preferences have ties         -> "weakly stable" always exists and this
//     still works after breaking ties arbitrarily; "strongly stable" needs a
//     different algorithm.
// ============================================================================
const int MAXN = 2005;

int n;
int pref[MAXN][MAXN];       // pref[p][k] = p's k-th choice acceptor
int rnk[MAXN][MAXN];        // rnk[a][p]  = p's position in acceptor a's list
int matchA[MAXN];           // acceptor a's current proposer, -1 if free
int matchP[MAXN];           // proposer p's current acceptor, -1 if free

// build rnk from the acceptors' preference lists
void build_rank(int aprefs[MAXN][MAXN]) {
    for (int a = 0; a < n; a++)
        for (int k = 0; k < n; k++) rnk[a][aprefs[a][k]] = k;   // invert once
}

void stable_matching() {
    vector<int> nxt(n, 0);                    // how many proposals p has made
    for (int i = 0; i < n; i++) matchA[i] = matchP[i] = -1;
    queue<int> free_;
    for (int p = 0; p < n; p++) free_.push(p);
    while (!free_.empty()) {
        int p = free_.front(); free_.pop();
        if (nxt[p] >= n) continue;            // exhausted (only if incomplete)
        int a = pref[p][nxt[p]++];            // propose to the next one down
        if (matchA[a] == -1) {
            matchA[a] = p, matchP[p] = a;
        } else if (rnk[a][p] < rnk[a][matchA[a]]) {
            int old = matchA[a];              // a prefers p: dump `old`
            matchP[old] = -1;
            free_.push(old);
            matchA[a] = p, matchP[p] = a;
        } else {
            free_.push(p);                    // rejected, try again lower down
        }
    }
}

// O(n^2) verification: no pair strictly prefers each other to their partners
bool is_stable() {
    for (int p = 0; p < n; p++)
        for (int a = 0; a < n; a++) {
            if (matchP[p] == a) continue;
            // does p prefer a to its own partner?
            int pa = -1, pcur = -1;
            for (int k = 0; k < n; k++) {
                if (pref[p][k] == a) pa = k;
                if (pref[p][k] == matchP[p]) pcur = k;
            }
            if (pa > pcur) continue;                        // p is content
            if (rnk[a][p] < rnk[a][matchA[a]]) return false; // and a agrees
        }
    return true;
}
