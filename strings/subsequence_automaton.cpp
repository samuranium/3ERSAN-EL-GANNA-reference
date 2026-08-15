// ============================================================================
// SUBSEQUENCE AUTOMATON -- "is t a subsequence of s" in O(|t|), any number of t
// ----------------------------------------------------------------------------
//     nxt[i][c] = the smallest index j >= i with s[j] == c, or n if none
//
// Build it right-to-left in O(n * sigma). Then matching is a greedy walk: for
// each character of t, jump; if you land on n, t is not a subsequence. The
// greedy is optimal because taking the EARLIEST occurrence never loses -- any
// later match leaves a suffix that is a subset of what the earliest leaves.
//
// One two-line table replaces a two-pointer scan per query. With q queries the
// two-pointer version is O(q * n); this is O(n*sigma + sum|t|).
//
// ############################################################################
// #  THE TABLE HAS n+1 ROWS AND ROW n IS THE DEAD STATE
// #
// #  nxt[n][c] = n for every c. Sizing it n rows means the last character of
// #  s is unreachable and every query that ends exactly at the end of s is
// #  reported as a non-subsequence -- a bug that only fires on the maximal
// #  matches, which random tests rarely produce.
// ############################################################################
//
// PITFALLS:
//   Memory is (n+1) * sigma ints. n = 1e6 over 26 letters is 104 MB -- that is
//     the real ceiling. Big alphabet: compress, or fall back to per-character
//     sorted position lists + binary search (O(log) per step, O(n) memory).
//   `nxt[i][c]` is defined for i in [0, n]; the walk starts at 0 and the
//     answer is "t is a subsequence iff the walk never reaches n".
//   This is about SUBSEQUENCES. Contiguous substrings are KMP / Z / SAM, and
//     confusing the two is the most common misroute in this area.
//   The DISTINCT-subsequence count below is where the automaton and the
//     last-occurrence DP meet: the subtraction is exactly the automaton's
//     determinism made arithmetic.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   is t a subsequence of s, many t                is_subseq()
//   how many of the queries are subsequences        same, count them
//   shortest string that is NOT a subsequence       BFS/DP on the automaton --
//                                                   shortest_non_subseq().
//                                                   The answer has length
//                                                   O(log_sigma n)
//   lexicographically smallest such string          same DP, pick the smallest
//                                                   character achieving the
//                                                   optimum
//   count DISTINCT subsequences of s                count_distinct() -- the
//                                                   last-occurrence
//                                                   subtraction
//   count distinct subsequences of length k         add a length dimension
//   k-th distinct subsequence in lex order          descend using the counts
//                                                   from count_distinct()
//   number of times t appears as a subsequence      a different DP:
//                                                   dp[i][j] += dp[i-1][j-1]
//                                                   -- NOT this automaton
//   count subsequences matching a regular rule      product DP: this automaton
//                                                   x the rule's DFA
//   "erase characters to reach t"                   |s| - |t| if t is a
//                                                   subsequence, else
//                                                   impossible
//
// NOT THIS:
//   longest common SUBSEQUENCE       -> LCS DP; the automaton does not help.
//   contiguous substring questions   -> suffix automaton / KMP.
//   t is fixed and s is huge         -> one two-pointer pass, no table.
// ============================================================================
const int SIG = 26;

vector<array<int, SIG>> build_subseq_automaton(const string &s) {
    int n = s.size();
    vector<array<int, SIG>> nxt(n + 1);
    for (int c = 0; c < SIG; c++) nxt[n][c] = n;        // the dead state
    for (int i = n - 1; i >= 0; i--) {
        nxt[i] = nxt[i + 1];
        nxt[i][s[i] - 'a'] = i;                          // earliest is here
    }
    return nxt;
}

bool is_subseq(vector<array<int, SIG>> &nxt, const string &t) {
    int n = (int)nxt.size() - 1, i = 0;
    for (char ch : t) {
        int j = nxt[i][ch - 'a'];
        if (j == n) return false;
        i = j + 1;                                       // consume it
    }
    return true;
}

// shortest string over the alphabet that is NOT a subsequence of s.
// dp[i] = shortest length still needed, starting the walk at position i.
string shortest_non_subseq(vector<array<int, SIG>> &nxt) {
    int n = (int)nxt.size() - 1;
    vector<int> dp(n + 2);
    dp[n] = 1;                                           // at the dead state
    for (int i = n - 1; i >= 0; i--) {
        dp[i] = INT_MAX;
        for (int c = 0; c < SIG; c++) {
            int j = nxt[i][c];
            int cost = (j == n) ? 1 : dp[j + 1] + 1;
            dp[i] = min(dp[i], cost);
        }
    }
    string res;
    int i = 0;
    while (true) {
        for (int c = 0; c < SIG; c++) {                  // smallest char first
            int j = nxt[i][c];
            int cost = (j == n) ? 1 : dp[j + 1] + 1;
            if (cost == dp[i]) {
                res.push_back('a' + c);
                if (j == n) return res;
                i = j + 1;
                break;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// COUNT DISTINCT SUBSEQUENCES -- the last-occurrence subtraction
//
//     dp[i] = 2 * dp[i-1] - dp[last[c] - 1]      (c = s[i-1])
//
// Doubling counts "extend everything by c" plus "keep everything"; the
// subtraction removes exactly the set that the PREVIOUS occurrence of c
// already produced. dp[0] = 1 counts the empty subsequence -- subtract 1 at
// the end if the problem wants non-empty.
//
// Under a modulus the subtraction goes negative: add mod before storing.
////////////////////////////////////////////////////////////////////////////////
ll count_distinct(const string &s, ll mod) {
    int n = s.size();
    vector<ll> dp(n + 1, 0);
    vector<int> last(SIG, 0);                            // 0 = "never seen"
    dp[0] = 1;                                           // the empty string
    for (int i = 1; i <= n; i++) {
        int c = s[i - 1] - 'a';
        dp[i] = dp[i - 1] * 2 % mod;
        if (last[c]) dp[i] = (dp[i] - dp[last[c] - 1] % mod + mod) % mod;
        last[c] = i;
    }
    return dp[n];                                        // includes ""
}
