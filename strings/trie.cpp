// ============================================================================
// TRIE + DFS -- a prefix tree you can walk, O(|s|) per operation
// ----------------------------------------------------------------------------
// Every node is a distinct PREFIX of some inserted word. Two counters make
// everything below possible:
//     term[u]  how many inserted words END exactly at u
//     sub[u]   how many words live in u's whole subtree  (term + all below)
//
// sub[] is what turns the trie from "does this word exist" into an ordered
// container: k-th smallest, rank, and a DFS that visits words in lexicographic
// order all read sub[] and never scan.
//
// ############################################################################
// #  THE DFS VISITS term[] BEFORE THE CHILDREN
// #
// #  "ab" sorts before "abc". A word ending at u is a proper prefix of
// #  everything below u, so it is lexicographically smaller than all of them.
// #  Emitting term[u] after recursing produces an order that looks sorted on
// #  most tests and is wrong whenever one word is a prefix of another.
// ############################################################################
//
// ############################################################################
// #  erase() LEAVES DEAD NODES BEHIND
// #
// #  It decrements the counters but never frees anything, so a node can exist
// #  with sub == 0. EVERY traversal must skip `sub[v] == 0` subtrees, or a
// #  deleted word reappears as an empty branch and kth() walks into a dead end.
// #  Physically pruning is possible but rarely worth it -- the node count is
// #  bounded by total inserted length regardless.
// ############################################################################
//
// PITFALLS:
//   `t[u].nxt[c] = newnode();` is a reallocation hazard -- newnode() does a
//     push_back that can move the vector while t[u] is being addressed. Take
//     the id into a local first (done below). C++17 sequences it safely, but
//     the two-line form is immune and obvious.
//   Reserve the pool: total inserted length + 1 nodes, worst case.
//   Memory is nodes * A ints. sum|s| = 1e6 over 26 letters is ~104 MB -- that
//     is the real ceiling. Large or sparse alphabets: swap array for a map and
//     accept the log.
//   erase() on an absent word must be a no-op, not a decrement -- check first.
//   The empty string is a legal word here; it lives as term[0].
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS -- what the DFS and sub[] buy you
// ----------------------------------------------------------------------------
//   does word s exist / how many copies      count()
//   how many words start with prefix p       count_prefix()  -- one walk, O(|p|)
//   all words in lexicographic order         collect()       -- the DFS
//   k-th smallest word                       kth()           -- descend on sub[]
//   rank of s (how many words are < s)       rank_of()
//   longest stored word that prefixes q      longest_word_prefix()
//   number of distinct prefixes              live node count
//   longest common prefix of ALL words       walk down while sub[u] == sub[0]
//                                            and term[u] == 0
//   autocomplete: first k words under p      walk to p, then a bounded DFS
//   sort strings without comparisons         insert all, then collect()
//   count distinct words                     nodes with term > 0
//   "does any stored word prefix q"          longest_word_prefix() >= 0 --
//                                            the dictionary-matching primitive
//   many patterns, search a TEXT             NOT this -- add fail links and it
//                                            becomes strings/aho_corasick.cpp
//   max XOR pair / bitwise queries           DS/Trie/binary_trie.cpp
//   persistent version (queries over a       copy the touched path on insert,
//     prefix of the inserts)                 one root per version
//
// NOT THIS: substrings of one fixed text -> suffix automaton / suffix array.
//   A trie stores whole words; it knows nothing about their substrings.
// ============================================================================
struct Trie {
    static const int A = 26;
    struct Node {
        array<int, A> nxt;
        int term, sub;
    };
    vector<Node> t;

    Trie(int reserve_len = 0) { t.reserve(reserve_len + 1); newnode(); }

    int newnode() {
        Node n;
        n.nxt.fill(-1), n.term = 0, n.sub = 0;
        t.push_back(n);
        return (int)t.size() - 1;
    }
    void insert(const string &s) {
        int u = 0;
        t[0].sub++;
        for (char ch : s) {
            int c = ch - 'a';
            if (t[u].nxt[c] < 0) {
                int v = newnode();          // separate line: newnode may realloc
                t[u].nxt[c] = v;
            }
            u = t[u].nxt[c];
            t[u].sub++;
        }
        t[u].term++;
    }
    // walk to the node for p, or -1 if that prefix is absent / dead
    int walk(const string &p) const {
        int u = 0;
        for (char ch : p) {
            int c = ch - 'a';
            if (t[u].nxt[c] < 0) return -1;
            u = t[u].nxt[c];
            if (!t[u].sub) return -1;       // dead branch left by erase()
        }
        return u;
    }
    int count(const string &s) const {
        int u = walk(s);
        return u < 0 ? 0 : t[u].term;
    }
    int count_prefix(const string &p) const {
        int u = walk(p);
        return u < 0 ? 0 : t[u].sub;
    }
    bool erase(const string &s) {           // one copy; false if absent
        if (!count(s)) return false;
        int u = 0;
        t[0].sub--;
        for (char ch : s) {
            u = t[u].nxt[ch - 'a'];
            t[u].sub--;
        }
        t[u].term--;
        return true;
    }
    // ---- DFS: every word, in lexicographic order ----
    void dfs(int u, string &cur, vector<string> &out) const {
        for (int i = 0; i < t[u].term; i++) out.push_back(cur);   // BEFORE children
        for (int c = 0; c < A; c++) {
            int v = t[u].nxt[c];
            if (v < 0 || !t[v].sub) continue;                     // skip dead
            cur.push_back('a' + c);
            dfs(v, cur, out);
            cur.pop_back();
        }
    }
    vector<string> collect() const {
        vector<string> out;
        string cur;
        dfs(0, cur, out);
        return out;
    }
    // all words under a prefix, lexicographic. Empty if the prefix is absent.
    vector<string> collect_prefix(const string &p) const {
        int u = walk(p);
        vector<string> out;
        if (u < 0) return out;
        string cur = p;
        dfs(u, cur, out);
        return out;
    }
    // k-th smallest word, 1-indexed, counting duplicates. "" if k is out of range.
    string kth(int k) const {
        if (k < 1 || k > t[0].sub) return "";
        int u = 0;
        string res;
        while (true) {
            if (k <= t[u].term) return res;
            k -= t[u].term;
            for (int c = 0; c < A; c++) {
                int v = t[u].nxt[c];
                if (v < 0 || !t[v].sub) continue;
                if (k <= t[v].sub) { res.push_back('a' + c), u = v; break; }
                k -= t[v].sub;
            }
        }
    }
    // how many stored words are strictly less than s
    int rank_of(const string &s) const {
        int u = 0, r = 0;
        for (char ch : s) {
            int c = ch - 'a';
            r += t[u].term;                       // proper prefixes of s are smaller
            for (int i = 0; i < c; i++) {
                int v = t[u].nxt[i];
                if (v >= 0) r += t[v].sub;        // whole smaller-letter subtrees
            }
            if (t[u].nxt[c] < 0 || !t[t[u].nxt[c]].sub) return r;
            u = t[u].nxt[c];
        }
        return r;
    }
    // length of the longest stored word that is a prefix of q; -1 if none
    int longest_word_prefix(const string &q) const {
        int u = 0, best = -1;
        for (int i = 0;; i++) {
            if (t[u].term) best = i;
            if (i == (int)q.size()) break;
            int c = q[i] - 'a';
            if (t[u].nxt[c] < 0 || !t[t[u].nxt[c]].sub) break;
            u = t[u].nxt[c];
        }
        return best;
    }
    // number of distinct non-empty prefixes currently stored
    int distinct_prefixes() const {
        int r = 0;
        for (int u = 1; u < (int)t.size(); u++) r += (t[u].sub > 0);
        return r;
    }
};
