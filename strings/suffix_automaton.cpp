// ============================================================================
// SUFFIX AUTOMATON -- every substring of s, as a DAG, in O(n * A)
// ----------------------------------------------------------------------------
// The minimal DFA accepting exactly the SUFFIXES of s. Its real value is that
// every PATH from the root spells a distinct substring, so counting paths
// counts substrings -- and it is built online, one character at a time.
//
// At most 2n-1 states and 3n-4 transitions, always. That bound is why it is
// safe at n = 1e6 where a suffix tree's constant would hurt.
//
// TWO STRUCTURES IN ONE, and most problems only need the second:
//   the AUTOMATON  (nxt[])   -- walk it to test/locate substrings
//   the LINK TREE  (link[])  -- suffix links form a tree on the states, and
//                               almost every counting problem is a DP on it
//
// ############################################################################
// #  WHAT A STATE ACTUALLY IS
// #
// #  A state holds an equivalence class of substrings that occur at exactly
// #  the same set of END POSITIONS. Within a state the lengths form a
// #  contiguous range  ( len[link[v]] + 1 .. len[v] ).
// #
// #  So state v contributes  len[v] - len[link[v]]  DISTINCT substrings, and
// #  all of them share one occurrence count. Nearly every formula below is
// #  that one sentence.
// ############################################################################
//
// ############################################################################
// #  CLONING IS NOT AN OPTIMISATION -- IT IS THE ALGORITHM
// #
// #  When st[p].len + 1 != st[q].len, state q mixes lengths that no longer
// #  share end positions, so it must be SPLIT. Skip the clone and the
// #  automaton still runs, still accepts, and silently reports wrong counts.
// #  The clone copies len from p (not q), inherits q's link and transitions,
// #  and then q's link points at it.
// ############################################################################
//
// PITFALLS:
//   Reserve 2*n states, not n. Cloning is why.
//   cnt[] must be seeded 1 on the non-clone states only, then pushed up the
//     link tree in order of decreasing len. Counting sort on len is O(n);
//     std::sort is fine too but is the usual reason this is "slow".
//   The clone's cnt starts at 0. Seeding it 1 double-counts.
//   Transitions here are array<int,26>: 2n * 26 ints, ~200 MB at n = 1e6.
//     For large n or big alphabets switch to a map and accept the log.
//   Build over the string in order; extend() is online, so prefixes work, but
//     you cannot delete characters.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS -- nearly all are one sweep over the link tree
// ----------------------------------------------------------------------------
//   is p a substring of s                walk nxt from root, O(|p|)
//   # of DISTINCT substrings             sum of len[v] - len[link[v]]
//   total LENGTH of distinct substrings  sum of the arithmetic series per state
//   # occurrences of a substring p       walk to a state, read cnt[]
//   # occurrences of EVERY substring     cnt[] by link-tree DP
//   first occurrence position of p       firstpos[v] - |p| + 1
//   all occurrence positions             walk the link tree below the state
//   k-th smallest distinct substring     paths[] DP over the DAG, then descend
//   longest common substring of s and t  feed t through s's automaton, track
//                                        the current matched length
//   LCS of many strings                  same, intersect per-state maxima
//   smallest / largest cyclic shift      build on s+s, walk greedily n steps
//   longest repeated substring           max len[v] over states with cnt >= 2
//   count substrings occurring >= k      sum over states with cnt >= k
//
// NOT THIS: comparing arbitrary substrings, or LCP of two suffixes -> suffix
//   array (strings/cleansuf.cpp). Multi-pattern search against a fixed
//   dictionary -> Aho-Corasick. One pattern -> KMP/Z.
// ============================================================================
struct SAM {
    static const int A = 26;
    struct State {
        int len, link;
        array<int, A> nxt;
        bool clone;
    };
    vector<State> st;
    int last;

    SAM(int reserve_n = 0) {
        st.reserve(2 * reserve_n + 5);
        st.push_back({0, -1, {}, false});
        st[0].nxt.fill(0);
        st[0].nxt.fill(-1);
        last = 0;
    }
    void extend(char ch) {
        int c = ch - 'a', cur = st.size();
        st.push_back({st[last].len + 1, -1, {}, false});
        st[cur].nxt.fill(-1);
        int p = last;
        while (p != -1 && st[p].nxt[c] == -1) st[p].nxt[c] = cur, p = st[p].link;
        if (p == -1) st[cur].link = 0;
        else {
            int q = st[p].nxt[c];
            if (st[p].len + 1 == st[q].len) st[cur].link = q;
            else {                                    // SPLIT -- see the box
                int clone = st.size();
                st.push_back({st[p].len + 1, st[q].link, st[q].nxt, true});
                while (p != -1 && st[p].nxt[c] == q) st[p].nxt[c] = clone, p = st[p].link;
                st[q].link = st[cur].link = clone;
            }
        }
        last = cur;
    }
    void build(const string &s) { for (char c : s) extend(c); }

    // # of distinct non-empty substrings
    ll distinct_substrings() const {
        ll r = 0;
        for (int v = 1; v < (int)st.size(); v++) r += st[v].len - st[st[v].link].len;
        return r;
    }
    // total length of all distinct substrings
    ll total_length() const {
        ll r = 0;
        for (int v = 1; v < (int)st.size(); v++) {
            ll lo = st[st[v].link].len + 1, hi = st[v].len;
            r += (lo + hi) * (hi - lo + 1) / 2;
        }
        return r;
    }
    // occurrence count of every state, by link-tree DP in decreasing len order
    vector<ll> occurrences() const {
        int n = st.size();
        vector<ll> cnt(n, 0);
        for (int v = 1; v < n; v++) cnt[v] = st[v].clone ? 0 : 1;
        vector<int> order(n), bucket(st[last].len + 2, 0);   // counting sort on len
        for (int v = 0; v < n; v++) bucket[st[v].len]++;
        for (int i = 1; i < (int)bucket.size(); i++) bucket[i] += bucket[i - 1];
        for (int v = n - 1; v >= 0; v--) order[--bucket[st[v].len]] = v;
        for (int i = n - 1; i > 0; i--) {                    // longest first
            int v = order[i];
            if (st[v].link >= 0) cnt[st[v].link] += cnt[v];
        }
        return cnt;
    }
    // state reached by p, or -1 if p is not a substring
    int walk(const string &p) const {
        int v = 0;
        for (char ch : p) {
            int c = ch - 'a';
            if (st[v].nxt[c] == -1) return -1;
            v = st[v].nxt[c];
        }
        return v;
    }
    bool contains(const string &p) const { return walk(p) != -1; }
};
