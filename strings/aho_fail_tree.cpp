// ============================================================================
// AHO FAIL TREE + EULER TOUR + BIT -- "how often does pattern i occur in j"
// ----------------------------------------------------------------------------
// The failure links form a TREE rooted at node 0. Its defining property:
//
//     pattern p occurs at a position ending in state u
//       <=>  p's terminal node is an ANCESTOR of u in the fail tree
//
// So "how many times does p occur in the text" = how many of the text's
// visited states lie in p's SUBTREE. Euler-tour the fail tree, and a subtree
// becomes a range:
//
//     walking the text     +1 at tin[state] in a BIT
//     answer for p         range sum over [tin[p], tout[p])
//
// strings/aho_corasick.cpp already counts all patterns at once with a reverse
// BFS -- use that when you want every count after reading the whole text. Use
// THIS when the queries are interleaved with the walk, or when the "text" is
// itself one of the patterns.
//
// ############################################################################
// #  PATTERN-INSIDE-PATTERN NEEDS ONE TRIE, NOT ONE AUTOMATON PER PAIR
// #
// #  To answer "how many times does pattern i occur inside pattern j" for
// #  many pairs: build ONE Aho over all patterns. Then walk j's own path down
// #  the trie, adding +1 at each node's tin. Query i's subtree sum. Undo by
// #  walking j's path again with -1.
// #
// #  Cost is O(|j| log n) per j, and you never build a second automaton.
// #  Building one automaton per pair is O(sum |p| * k) and is the trap.
// ############################################################################
//
// ############################################################################
// #  THE +1 GOES AT THE TRIE PATH NODES, NOT THE GOTO-AUTOMATON STATES
// #
// #  When j is one of the patterns, walk the raw TRIE children down j -- those
// #  are exactly the prefixes of j. Using the fallback-resolved goto here
// #  visits states that are suffixes of prefixes, and counts occurrences that
// #  are not inside j at all.
// #
// #  When j is an arbitrary TEXT, the opposite holds: you must use the
// #  fallback-resolved goto, because the text's prefixes are not trie nodes.
// #  Two different questions, two different walks.
// ############################################################################
//
// PITFALLS:
//   Build the fail tree from link[] AFTER build(), and Euler tour it -- the
//     BFS order is not a DFS order and cannot be used for subtree ranges.
//   The root (node 0) is the whole tree; its subtree sum counts everything.
//   Undo with -1 rather than clearing the BIT, or the total is O(k * n).
//   tout is EXCLUSIVE below; query [tin, tout - 1] inclusive on the BIT.
//   Duplicate patterns share a terminal node -- keep a list of ids per node,
//     and answer both from the same subtree sum.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   occurrences of every pattern in one text        reverse BFS is cheaper --
//                                                   strings/aho_corasick.cpp
//   occurrences of pattern i inside pattern j        this -- one trie, Euler
//                                                    tour, BIT
//   ... for many (i, j) pairs                        this, offline by j
//   occurrences in a PREFIX of the text              add +1 as you walk and
//                                                    query at each step
//   "after reading t characters, how many p so far"   same, online
//   count of patterns matching at each position       walk, query the
//                                                    root-to-state path count
//                                                    (= depth in terminal
//                                                    nodes)
//   patterns arriving online                          logarithmic rebuilding:
//                                                    O(log n) automata of
//                                                    sizes 2^k
//   dictionary matching where patterns are deleted     BIT over time, or
//                                                    rebuild
//
// NOT THIS:
//   one pattern, one text  -> KMP. strings/Z kmp.cpp.
//   substrings of a single fixed text -> suffix automaton.
//   you only need the totals at the end -> aho_corasick.cpp count_all().
// ============================================================================
// needs the Aho struct from strings/aho_corasick.cpp (nxt, link, term, node(),
// add(), build()). This file adds the fail tree on top.

vector<int> ftree[400005];       // children in the FAIL tree
int tin_[400005], tout_[400005], timer_;
int bitf[400005];                // BIT over the Euler tour

void bit_upd(int i, int v) { for (i++; i < 400005; i += i & -i) bitf[i] += v; }
int  bit_qry(int i) { int s = 0; for (i++; i > 0; i -= i & -i) s += bitf[i]; return s; }
int  bit_range(int l, int r) { return r < l ? 0 : bit_qry(r) - (l ? bit_qry(l - 1) : 0); }

// iterative Euler tour -- the trie can be 1e6 deep and recursion will not survive
void euler(int root, int n) {
    timer_ = 0;
    vector<pair<int, int>> st{{root, 0}};
    tin_[root] = timer_++;
    while (!st.empty()) {
        auto &[u, i] = st.back();
        if (i < (int)ftree[u].size()) {
            int v = ftree[u][i++];
            tin_[v] = timer_++;
            st.push_back({v, 0});
        } else {
            tout_[u] = timer_;               // EXCLUSIVE
            st.pop_back();
        }
    }
    (void)n;
}
// build ftree from link[]. Node 0 is the root and links to itself; skip it.
void build_fail_tree(vector<int> &link) {
    int n = link.size();
    for (int i = 0; i < n; i++) ftree[i].clear();
    for (int v = 1; v < n; v++) ftree[link[v]].push_back(v);
    euler(0, n);
}

// occurrences of the pattern whose terminal node is `p`, among everything
// currently added to the BIT
int occurrences(int p) { return bit_range(tin_[p], tout_[p] - 1); }

////////////////////////////////////////////////////////////////////////////////
// PATTERN INSIDE PATTERN -- the whole point of this file.
//
//   term_node[i] = the trie node where pattern i ends
//
// For each j: walk j down the RAW TRIE (its own prefixes), +1 at each node,
// answer every query (i, j) with occurrences(term_node[i]), then walk again
// with -1. See the second box for why the raw trie and not the goto automaton.
//
// Usage (A is the Aho from strings/aho_corasick.cpp, already built):
//
//   build_fail_tree(A.link);
//   for each j:
//       int u = 0;
//       for (char ch : pat[j]) { u = trie_child[u][ch-'a']; bit_upd(tin_[u], 1); }
//       for each query (i, j)  ans = occurrences(term_node[i]);
//       u = 0;
//       for (char ch : pat[j]) { u = trie_child[u][ch-'a']; bit_upd(tin_[u], -1); }
//
// `trie_child` must be a COPY of nxt[][] taken BEFORE build() overwrites the
// zeros with fallback edges -- that is the single thing to remember here.
////////////////////////////////////////////////////////////////////////////////
