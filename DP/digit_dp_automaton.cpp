// ============================================================================
// DIGIT DP + AUTOMATON -- count numbers in [L, R] satisfying a rule
// ----------------------------------------------------------------------------
// Same frame as DP/digit dp.cpp, but the state carries an AUTOMATON STATE that
// the transition advances. That single extra dimension covers divisibility,
// forbidden substrings, digit-sum conditions and KMP/Aho matching -- rules
// that cannot be expressed by "tight" and a counter alone.
//
//   dp[pos][tight][started][autstate]
//
// ############################################################################
// #  THE FOUR FLAGS, AND WHICH ONES YOU ACTUALLY NEED
// #
// #   pos      which digit, left to right.                      always
// #   tight    still equal to the bound's prefix? If yes the     always
// #            next digit is capped at bound[pos].
// #   started  have we placed a nonzero digit yet? Needed        only if
// #            whenever LEADING ZEROS must not count as real     leading
// #            digits -- "digit 0 appears k times", "the number  zeros
// #            has d digits". NOT needed for pure divisibility.  matter
// #   aut      the automaton state.                              this file
// #
// #  Forgetting `started` is the classic wrong answer: 007 is counted as
// #  containing two zeros. Adding it when it is not needed doubles the state
// #  space for nothing.
// ############################################################################
//
// ############################################################################
// #  COUNT [L, R] AS f(R) - f(L-1), NOT AS ONE DP
// #
// #  Two bounds in one DP needs a second tight flag and four times the states.
// #  f(X) = "how many in [0, X]" is the same function twice.
// #  L = 0 needs care: f(-1) = 0. For big-number L given as a string, subtract
// #  one as a string, or handle L itself separately.
// ############################################################################
//
// PITFALLS:
//   Memo only the states where tight == false. Tight states are visited once
//     each, and caching them across different bounds is the classic
//     cross-contamination bug. (The version below memoises on !tight only.)
//   Reset the memo per bound, or stamp it with a test-case id -- DP/digit
//     dp.cpp uses the stamp trick.
//   The automaton must be TOTAL: every (state, digit) pair needs a target.
//     A missing edge is a dead state, not a crash.
//   Size the memo by the real bounds. `DP/digit dp.cpp` has a mismatch between
//     its dp and vis dimensions -- re-derive them per problem.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS -- what the automaton is, per problem type
// ----------------------------------------------------------------------------
//   divisible by m                     state = value mod m; step:
//                                      (s*10 + d) % m
//   digit sum equals / divisible by k  state = running sum (or sum mod k)
//   does NOT contain "13" / "666"      state = KMP prefix-function position in
//                                      the forbidden pattern; reject on a full
//                                      match -- strings/Z kmp.cpp
//   avoids ANY of a set of patterns    state = Aho-Corasick node; reject when
//                                      outlink marks a terminal --
//                                      strings/aho_corasick.cpp
//   no two adjacent equal digits       state = previous digit
//   digits non-decreasing              state = previous digit, cap the range
//   contains digit d at least k times  state = min(count, k)
//   alternating parity                 state = parity of the last digit
//   the number itself is a palindrome  NOT an automaton -- fix the length and
//                                      build the halves directly
//   sum of the numbers, not the count  carry (count, sum) as a pair and
//                                      combine: sum = sum_child + d*10^pos*cnt
//
// NOT THIS: if the property depends on the number as a whole in a non-local
//   way (primality, "is a perfect square"), digit DP does not apply.
// ============================================================================
const int DPOS = 20;          // max digits
const int DAUT = 105;         // max automaton states -- size per problem

int MOD_M;                    // example rule: divisible by MOD_M
ll memo[DPOS][2][DAUT];
bool seen[DPOS][2][DAUT];
string bound_;

// EXAMPLE automaton: state = value mod MOD_M. Replace this and DAUT only.
inline int step(int s, int d) { return (s * 10 + d) % MOD_M; }
inline bool accept(int s, bool started) { return started && s == 0; }

ll go(int pos, bool tight, bool started, int aut) {
    if (pos == (int)bound_.size()) return accept(aut, started);
    if (!tight && seen[pos][started][aut]) return memo[pos][started][aut];
    int hi = tight ? bound_[pos] - '0' : 9;
    ll res = 0;
    for (int d = 0; d <= hi; d++) {
        bool nstarted = started || d > 0;
        int naut = nstarted ? step(aut, d) : aut;     // leading zeros do not advance
        res += go(pos + 1, tight && d == hi, nstarted, naut);
    }
    if (!tight) seen[pos][started][aut] = true, memo[pos][started][aut] = res;
    return res;
}
// how many x in [0, X] satisfy the rule
ll count_upto(const string &X) {
    bound_ = X;
    memset(seen, 0, sizeof seen);
    return go(0, true, false, 0);
}
// [L, R] as f(R) - f(L-1); pass L already decremented -- see the box
ll count_range(const string &Lminus1, const string &R) {
    return count_upto(R) - count_upto(Lminus1);
}
