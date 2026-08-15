// ============================================================================
// EERTREE (palindromic tree) -- all DISTINCT palindromic substrings, O(n)
// ----------------------------------------------------------------------------
// One node per DISTINCT palindromic substring. The whole structure is linear
// because a string of length n contains at most n distinct palindromes -- that
// bound IS the reason this works, and it is what Manacher does not give you:
// Manacher counts ALL palindromic substrings (O(n^2) of them), never distinct.
//
// Two roots:
//     node 1  the imaginary palindrome of length -1
//     node 0  the empty palindrome, length 0, link -> node 1
//
// `link[v]` = the longest PROPER palindromic SUFFIX of v. Adding character
// s[i] walks the suffix-link chain from `last` until s[i - len(t) - 1] == s[i],
// which is the extension test.
//
// ############################################################################
// #  THE LENGTH -1 ROOT IS NOT A HACK, IT IS LOAD-BEARING
// #
// #  Extending the -1 node produces the single-character palindrome: its
// #  length becomes -1 + 2 = 1. Without it the tree never creates length-1
// #  nodes, and the whole structure silently reports only even palindromes.
// #  Allocate n+2 nodes and build BOTH roots before the first character.
// ############################################################################
//
// ############################################################################
// #  A NEW NODE'S LINK NEEDS A SECOND WALK
// #
// #  After creating `cur` you must find its longest proper palindromic suffix
// #  by continuing the walk from link[t], NOT by reusing t. And the guard
// #  `if (len[cur] == 1) link[cur] = 0;` is required -- otherwise a
// #  single-character node links to itself and every later traversal hangs.
// ############################################################################
//
// PITFALLS:
//   cnt[v] counts only the occurrences ENDING where v was last created. The
//     true occurrence count needs one backward pass propagating cnt along the
//     links, in decreasing node order -- node ids are created in increasing
//     length-of-creation order, so a plain reverse loop is correct.
//   `num[v]` (number of palindromic suffixes of v) is `num[link[v]] + 1`, and
//     it is what makes "count palindromic suffixes at each position" O(1).
//   The number of NODES is the number of distinct palindromes; subtract the
//     two roots.
//   Walking the suffix-link chain per position is amortised O(1) only for
//     CONSTRUCTION. Doing a DP along that chain is O(n) per position in the
//     worst case (`a^n`) -- that is what series links fix, see below.
//   s must be indexable with the current position; keep the string, not just
//     the characters.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   number of DISTINCT palindromic substrings      tot - 2
//   occurrences of each distinct palindrome        cnt[] after propagate()
//   the most frequent palindrome                   max over len[v] * cnt[v]
//   number of palindromic substrings, total        sum of cnt[] -- Manacher
//                                                  also does this, cheaper
//   palindromes ENDING at position i               the suffix-link chain from
//                                                  last, or num[last] for the
//                                                  count alone
//   longest palindromic suffix at each position    len[last] after each add
//   min palindromic factorisation                  series links, below
//   palindromes common to two strings              build over s, then keep
//                                                  adding t without resetting
//                                                  the node pool
//   k-th distinct palindrome in lex order          DFS the DIRECT-EDGE tree
//                                                  (nxt[]), not the link tree
//
// NOT THIS:
//   longest palindromic SUBSTRING only          -> Manacher, shorter and O(n).
//   longest palindromic SUBSEQUENCE             -> LCS(s, rev s) / interval DP.
//                                                  Not a palindrome structure
//                                                  at all.
//   "is s[l..r] a palindrome"                   -> forward vs reverse hash.
// ============================================================================
struct Eertree {
    static const int A = 26;
    vector<array<int, A>> nxt;
    vector<int> len, link, cnt, num;
    vector<char> s;
    int last, tot;

    Eertree(int reserve_n = 0) {
        nxt.reserve(reserve_n + 2), len.reserve(reserve_n + 2);
        newnode(-1);              // node 0 in the pool: the -1 root
        newnode(0);               // node 1 in the pool: the empty root
        link[0] = 0, link[1] = 0; // both roots fall back to the -1 root
        last = 1, tot = 2;
        s.push_back(-1);          // sentinel so s[i-len-1] is always in range
    }
    int newnode(int l) {
        array<int, A> z; z.fill(0);
        nxt.push_back(z), len.push_back(l), link.push_back(0);
        cnt.push_back(0), num.push_back(0);
        return (int)len.size() - 1;
    }
    // walk down the suffix links from v until s[i - len(v) - 1] == s[i]
    int getlink(int v, int i) {
        while (s[i - len[v] - 1] != s[i]) v = link[v];
        return v;
    }
    // returns true if a NEW distinct palindrome was created
    bool add(char ch) {
        s.push_back(ch);
        int i = (int)s.size() - 1, c = ch - 'a';
        int t = getlink(last, i);
        bool fresh = false;
        if (!nxt[t][c]) {
            int cur = newnode(len[t] + 2);
            // second walk: the new node's longest proper palindromic suffix
            link[cur] = nxt[getlink(link[t], i)][c];
            if (len[cur] == 1) link[cur] = 1;      // must not self-link
            num[cur] = num[link[cur]] + 1;
            nxt[t][c] = cur;
            tot++, fresh = true;
        }
        last = nxt[t][c];
        cnt[last]++;
        return fresh;
    }
    int distinct() const { return tot - 2; }

    // true occurrence counts. Nodes were created in increasing order, and a
    // node's link is always an EARLIER node, so one reverse pass suffices.
    void propagate() {
        for (int v = (int)len.size() - 1; v >= 2; v--) cnt[link[v]] += cnt[v];
        cnt[0] = cnt[1] = 0;
    }
    ll total_palindromic_substrings() {            // counts duplicates
        ll r = 0;
        for (int v = 2; v < (int)len.size(); v++) r += cnt[v];
        return r;
    }
    ll best_len_times_occ() {
        ll r = 0;
        for (int v = 2; v < (int)len.size(); v++) r = max(r, (ll)len[v] * cnt[v]);
        return r;
    }
};

////////////////////////////////////////////////////////////////////////////////
// SERIES LINKS -- palindromic factorisation in O(n log n)
//
// The palindromic suffixes of any prefix form O(log n) ARITHMETIC PROGRESSIONS
// by length. `diff[v] = len[v] - len[link[v]]`; `slink[v]` jumps to the last
// node of the previous progression. Walking slink is O(log n) per position,
// where walking link is O(n).
//
// dp[i] = min pieces to split s[0..i-1] into palindromes. `g[v]` carries the
// running best for v's whole progression so each group costs O(1).
////////////////////////////////////////////////////////////////////////////////
struct PalFactor : Eertree {
    vector<int> diff, slink, g, dp;
    PalFactor(int n) : Eertree(n) {
        diff.assign(len.size(), 0), slink.assign(len.size(), 0);
        g.assign(len.size(), 0);
        dp.assign(n + 1, INT_MAX);
        dp[0] = 0;
    }
    void grow() {                                   // size the aux arrays
        diff.resize(len.size()), slink.resize(len.size()), g.resize(len.size());
    }
    // call once per character; returns dp[i] for the prefix of length i
    int step(char ch, int i) {                       // i = 1-based prefix length
        add(ch);
        grow();
        if (last >= 2 && diff[last] == 0) {          // freshly created node
            diff[last] = len[last] - len[link[last]];
            slink[last] = (diff[last] == diff[link[last]]) ? slink[link[last]]
                                                          : link[last];
        }
        for (int u = last; len[u] > 0; u = slink[u]) {
            g[u] = dp[i - len[slink[u]] - diff[u]];
            if (diff[u] == diff[link[u]]) g[u] = min(g[u], g[link[u]]);
            dp[i] = min(dp[i], g[u] + 1);
        }
        return dp[i];
    }
};
