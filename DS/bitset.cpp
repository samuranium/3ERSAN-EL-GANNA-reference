// ============================================================================
// BITSET -- the /64 button.  Everything here is O(n/64) instead of O(n).
// ----------------------------------------------------------------------------
// A bitset is not a data structure you design with; it is a CONSTANT FACTOR you
// apply to a loop you already have. If the inner loop is boolean -- an OR, an
// AND, a "did any of these happen" -- it collapses into whole 64-bit words.
//
// 1e9 boolean operations is a TLE. 1e9/64 = 1.6e7 is instant. That single
// division is why an O(n^2) or O(n^3) solution can pass at n = 5e4 or n = 2000.
//
// ############################################################################
// #  THE SIZE IS A COMPILE-TIME CONSTANT
// #
// #      bitset<100000> b;      OK        bitset<n> b;      COMPILE ERROR
// #
// #  Declare it at the maximum and use only the first n bits. If the bound is
// #  genuinely unknown until runtime, you need vector<uint64_t> and hand-rolled
// #  word operations -- at which point check whether you actually needed it.
// #
// #  _Find_first / _Find_next ARE GCC EXTENSIONS. They do not exist on MSVC.
// #  Codeforces, AtCoder, ICPC judges are all GCC, so this is safe in practice,
// #  but your local Visual Studio build will not compile.
// ############################################################################
//
// ----------------------------------------------------------------------------
// THE API -- in the order you will reach for it
// ----------------------------------------------------------------------------
//   bitset<N> b;              all zero
//   bitset<N> b(x);           from an integer, low bits first
//   bitset<N> b(string);      from "1011"; leftmost char is the HIGHEST index
//
//   b[i]                      read / write one bit, no bounds check
//   b.test(i)                 same but bounds-checked (throws)
//   b.set()      b.set(i)     all ones / one bit to 1
//   b.set(i, v)               one bit to v
//   b.reset()    b.reset(i)   all zero / one bit to 0
//   b.flip()     b.flip(i)    invert all / one
//
//   b.count()                 popcount, O(N/64)          <- the workhorse
//   b.any() b.none() b.all()  O(N/64)
//   b.size()                  N, the compile-time size
//
//   b._Find_first()           index of the lowest set bit, or N if empty
//   b._Find_next(i)           lowest set bit STRICTLY AFTER i, or N
//
//   & | ^ ~ << >> and their assign forms, all O(N/64)
//   b.to_ulong() / to_ullong()   throws if it does not fit
//   cout << b                 prints HIGHEST index first -- reads reversed
//
// ############################################################################
// #  _Find_first / _Find_next ARE THE REASON TO PREFER bitset OVER vector<bool>
// #
// #  Iterating only the SET bits costs O(k + N/64) for k set bits, not O(N):
// #
// #      for (int i = b._Find_first(); i < (int)b.size(); i = b._Find_next(i))
// #
// #  Both return b.size() when nothing is found -- that is the loop guard, and
// #  it is why the condition is `< b.size()` and never `!= -1`.
// #  A sparse bitset iterates almost for free. This turns "for each set bit,
// #  do something" from a full scan into a skip.
// ############################################################################
//
// ----------------------------------------------------------------------------
// WHEN TO REACH FOR IT -- the shapes that collapse
// ----------------------------------------------------------------------------
// [1] SUBSET-SUM / KNAPSACK FEASIBILITY (not counting, not maximising)
//       bitset<S+1> dp;  dp[0] = 1;
//       for (w : items) dp |= dp << w;         // O(n*S/64)
//     dp[s] is now "sum s is reachable". n = 100, S = 1e5 -> 1.5e5 word ops.
//     Bounded counts: binary-split the multiplicities (1,2,4,...) first.
//     Needs the COUNT of ways instead? Not this -- that is a real DP.
//     Subtracting as well as adding? Shift both ways and use an offset.
//
// [2] REACHABILITY / TRANSITIVE CLOSURE
//       for k: for i: if (reach[i][k]) reach[i] |= reach[k];
//     O(V^3/64) makes V = 2000 practical. Measured 114x faster than the plain
//     loop at V = 2000 -- see graph/shortest path/floyd warshall.cpp [3].
//     DISTANCES CANNOT be done this way; min-plus has no word equivalent.
//
// [3] GRAPH COUNTING WITH ADJACENCY BITSETS
//       common neighbours of u,v   (adj[u] & adj[v]).count()
//       triangles                  sum over edges of that, / 3
//       is this set a clique       for each u in S: (adj[u] & S) == S \ {u}
//     Max clique / max independent set by branch-and-bound over bitsets is the
//     standard way to push n from ~20 to ~60.
//
// [4] STRING MATCHING -- see the code below. O(|s|*|p|/64) with no automaton,
//     and it answers "how many occurrences in [l,r]" for free, which KMP does
//     not.
//
// [5] GAUSSIAN ELIMINATION OVER GF(2): rows are bitsets, elimination is one
//     xor per row -> O(n*m/64). See DP/sub-problems xorbasis.cpp for the
//     basis-only version.
//
// [6] SIEVE: bitset<N> is 8x less memory than bool[N], which matters at 1e8.
//     number theory/sieve.cpp already uses one.
//
// [7] LONGEST COMMON SUBSEQUENCE, bit-parallel (Hunt-Szymanski): O(n*m/64).
//     Rarely needed, but it is the answer when n*m is ~1e9.
//
// ----------------------------------------------------------------------------
// WHEN NOT TO
// ----------------------------------------------------------------------------
//   You need COUNTS or SUMS, not feasibility     -> ordinary DP
//   You need the bits to carry values            -> not boolean, no /64
//   The size is only known at runtime            -> see the box
//   n is small enough already                    -> do not add the complexity
//   You need per-bit updates in a query loop     -> a Fenwick/segment tree is
//                                                   O(log n) per op; a bitset
//                                                   rebuild is O(N/64) per op
//
// PITFALLS:
//   Shifts are O(N/64) even by one -- inside a loop over n items that is the
//     whole cost. Do not shift in an inner loop you could hoist.
//   cout << b prints index N-1 first. Debug output looks mirrored.
//   b[i] on i >= N is UB with no warning. test() throws instead.
//   Two bitsets must have the SAME N to combine; bitset<10> & bitset<20> does
//     not compile.
//   count() is O(N/64), not O(1). Caching it in a loop is a real speedup.
//   Memory is N/8 bytes: bitset<1e8> is 12.5 MB and fine; an ARRAY of them,
//     bitset<1e5> adj[1e5], is 1.25 GB and is not.
// ============================================================================
const int MAXB = 100005;

// ---------------------------------------------------------------------------
// [1] subset-sum feasibility -- can any subset of w[] reach exactly s?
// ---------------------------------------------------------------------------
bitset<MAXB> reach_sums(const vector<int> &w, int S) {
    bitset<MAXB> dp;
    dp[0] = 1;
    for (int x : w) dp |= dp << x;
    return dp;                       // dp[s] == 1  <=>  s is reachable
}

// ---------------------------------------------------------------------------
// iterate ONLY the set bits -- O(k + N/64), not O(N)
// ---------------------------------------------------------------------------
void for_each_set(const bitset<MAXB> &b) {
    for (int i = b._Find_first(); i < (int)b.size(); i = b._Find_next(i)) {
        (void)i;                     // i is a set bit
    }
}

// ---------------------------------------------------------------------------
// [4] STRING MATCHING WITH BITSETS  -- O(|s| * |p| / 64), no automaton
// ---------------------------------------------------------------------------
// bs[c][i] = 1 iff s[i] == c. Then a match of p starting at i requires
// s[i+j] == p[j] for every j, i.e. bit i set in every (bs[p[j]] >> j).
// AND them together and the surviving bits ARE the match positions.
//
// Beats KMP when you also need: occurrence COUNT in a range, all positions
// without walking, or many patterns against one fixed text.
// ---------------------------------------------------------------------------
bitset<MAXB> bs[26], occ;

void build_matcher(const string &s) {
    for (int c = 0; c < 26; c++) bs[c].reset();
    for (int i = 0; s[i]; i++) bs[s[i] - 'a'][i] = 1;
}
// after this, occ[i] == 1 means p occurs starting at i
void match(const string &p) {
    occ.set();
    for (int j = 0; p[j]; j++) occ &= (bs[p[j] - 'a'] >> j);
}
int count_all() { return occ.count(); }

// occurrences fully inside the 1-indexed window [l, r]
int count_in(int l, int r, int plen) {
    if (plen > r - l + 1) return 0;
    return (occ >> (l - 1)).count() - (occ >> (r - plen + 1)).count();
}
// every start position, cheaply
vector<int> all_positions() {
    vector<int> v;
    for (int i = occ._Find_first(); i < (int)occ.size(); i = occ._Find_next(i))
        v.push_back(i);
    return v;
}
