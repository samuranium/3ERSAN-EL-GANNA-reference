// ============================================================================
// AHO-CORASICK -- all patterns against one text, O(sum|p| * A + |s| + matches)
// ----------------------------------------------------------------------------
// A trie of the patterns, plus a failure link at every node pointing to the
// longest proper suffix that is still a trie node. Precomputing the transition
// for EVERY character turns the failure walk into a single array read, so the
// text is scanned once with no backtracking at all.
//
// It is KMP generalised from one pattern to a whole dictionary: link[] is the
// prefix function, and go[][] is the automaton KMP builds implicitly.
//
// ############################################################################
// #  TWO LINKS, AND THEY ARE NOT THE SAME
// #
// #    link[u]     longest proper suffix of u that is ANY node.
// #                Used to BUILD the automaton. Never walk it at query time.
// #
// #    outlink[u]  nearest node up the link chain that ENDS A PATTERN.
// #                Used to REPORT matches. Skips the nodes that are not
// #                terminal, so listing k matches costs O(k), not O(depth).
// #
// #  Conflate them and you either miss matches (using link and stopping at the
// #  first non-terminal) or go quadratic (walking link on every character).
// ############################################################################
//
// ############################################################################
// #  COUNTING vs LISTING -- pick before you write it
// #
// #  COUNT how many times each pattern occurs: do NOT walk outlink per
// #  character. Bump hit[u] once per text position, then push the totals up
// #  the link tree in REVERSE BFS order. O(|s| + nodes) regardless of matches.
// #
// #  LIST every (position, pattern) pair: you must walk outlink, and the cost
// #  is the number of matches, which can be O(|s| * #patterns) -- "aaa...a"
// #  against {a, aa, aaa, ...}. If the statement only wants counts, never list.
// ############################################################################
//
// PITFALLS:
//   Node 0 is both root and "null". A missing child is 0 AFTER build() only
//     because build() fills it with the failure target -- before build(), 0
//     genuinely means absent. Never query before building.
//   Alphabet A is a compile-time constant here. For large or sparse alphabets
//     swap array<int,A> for a map and lose the O(1) transition.
//   Duplicate patterns: term[] holds a LIST of ids, not one id. A dictionary
//     with repeats otherwise loses all but the last.
//   Memory is (sum of pattern lengths + 1) * A ints. 5e5 total length over 26
//     letters is ~52 MB -- that is the real limit, not time.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   does the text contain any pattern        walk, check outlink != 0
//   count occurrences of each pattern        count_all() below
//   all (position, pattern) pairs            list_matches() below
//   first position any pattern matches       walk until outlink != 0
//   longest / shortest pattern matching      compare depths at each hit
//   DP over strings avoiding a dictionary    the automaton IS the DP state:
//                                            dp[i][u] = ways to build length i
//                                            ending at node u, skipping any u
//                                            with outlink != 0. This is the
//                                            most common real use.
//   shortest string containing all patterns  dp[u][mask] over automaton node
//                                            and set of patterns seen, BFS
//   count strings of length L with >= 1      total - (strings avoiding all)
//     occurrence
//   many texts, one dictionary               build once, feed each text
//   one text, many queries of substrings     suffix automaton is usually the
//                                            better tool -- see strings/
//
// NOT THIS: a SINGLE pattern -> KMP or Z (strings/Z kmp.cpp), less code.
//   Substring queries about the TEXT itself -> suffix array / automaton.
//   Patterns arriving online -> Aho must be rebuilt; consider hashing.
// ============================================================================
struct Aho {
    static const int A = 26;
    vector<array<int, A>> nxt;     // after build(): full goto automaton
    vector<int> link, outlink;
    vector<vector<int>> term;      // pattern ids ending at this node
    vector<int> order;             // BFS order, for the reverse sweep
    vector<ll> hit;                // per-node hit counts from the last run

    int node() {
        nxt.push_back({}), link.push_back(0), outlink.push_back(0);
        term.push_back({});
        return nxt.size() - 1;
    }
    Aho() { node(); }              // node 0 = root

    void add(const string &p, int id) {
        int u = 0;
        for (char ch : p) {
            int c = ch - 'a';
            if (!nxt[u][c]) nxt[u][c] = node();
            u = nxt[u][c];
        }
        term[u].push_back(id);
    }
    // fills nxt into a total function, plus link and outlink. Call once.
    void build() {
        queue<int> q;
        order.clear();
        for (int c = 0; c < A; c++)
            if (nxt[0][c]) q.push(nxt[0][c]);
        while (q.size()) {
            int u = q.front(); q.pop();
            order.push_back(u);
            outlink[u] = term[link[u]].size() ? link[u] : outlink[link[u]];
            for (int c = 0; c < A; c++) {
                int v = nxt[u][c];
                if (!v) nxt[u][c] = nxt[link[u]][c];      // failure baked in
                else link[v] = nxt[link[u]][c], q.push(v);
            }
        }
        hit.assign(nxt.size(), 0);
    }
    int go(int u, char ch) const { return nxt[u][ch - 'a']; }

    // ---- COUNTING: O(|s| + nodes), independent of the number of matches ----
    // returns cnt[id] = number of occurrences of pattern id in s
    vector<ll> count_all(const string &s, int npat) {
        fill(hit.begin(), hit.end(), 0LL);
        int u = 0;
        for (char ch : s) u = go(u, ch), hit[u]++;
        vector<ll> cnt(npat, 0);
        for (int i = order.size() - 1; i >= 0; i--) {   // reverse BFS = leaves first
            int u2 = order[i];
            for (int id : term[u2]) cnt[id] += hit[u2];
            hit[link[u2]] += hit[u2];                   // push up the link tree
        }
        for (int id : term[0]) cnt[id] += hit[0];       // empty pattern, if any
        return cnt;
    }
    // ---- LISTING: O(number of matches). Can be quadratic; see the box. ----
    // calls f(end_position, pattern_id) for every occurrence
    template <class F>
    void list_matches(const string &s, F f) {
        int u = 0;
        for (int i = 0; i < (int)s.size(); i++) {
            u = go(u, s[i]);
            for (int v = term[u].size() ? u : outlink[u]; v; v = outlink[v])
                for (int id : term[v]) f(i, id);
        }
    }
};
