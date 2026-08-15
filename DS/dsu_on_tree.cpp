// ============================================================================
// DSU ON TREE ("small to large", "sack") -- one answer per subtree, O(n log n)
// ----------------------------------------------------------------------------
// The problem shape: for EVERY vertex, answer a question about the multiset of
// values in its subtree. Recomputing per subtree is O(n^2). The trick:
//
//     1. recurse into every LIGHT child, and ERASE its contribution
//     2. recurse into the HEAVY child (biggest subtree) and KEEP it
//     3. re-add all light subtrees, answer at v
//     4. if v is itself light, erase everything before returning
//
// Each vertex is added once per LIGHT EDGE on its path to the root, and there
// are at most log n of those (going through a light edge at least halves the
// subtree size). O(n log n) total add/remove calls, each O(1) with plain
// arrays -- no map, no log factor inside.
//
// ############################################################################
// #  THE ANSWER IS TAKEN AFTER STEP 3, NOT AFTER STEP 2
// #
// #  After the heavy recursion the counters hold only the heavy subtree. The
// #  light subtrees must be re-added BEFORE you read the answer. Reading it
// #  one line early gives a result that is correct for leaves and for chains
// #  -- which is exactly enough to pass the samples.
// ############################################################################
//
// ############################################################################
// #  `keep` DECIDES WHETHER YOU CLEAN UP, AND IT IS NOT "am I the root"
// #
// #  A call cleans up iff its own edge to the parent is LIGHT. The parent
// #  passes keep = false for light children and keep = true for the heavy one.
// #  Getting this backwards is O(n^2) with identical output, or leaves stale
// #  counts and gives wrong answers -- one bug, two symptoms.
// ############################################################################
//
// PITFALLS:
//   add()/remove() must be exact inverses. Anything not restorable in O(1)
//     (a sorted vector, a max that cannot be undone) does not belong here --
//     use segment tree merging instead.
//   Erasing by "clear the whole array" is O(sigma) per call and kills the
//     bound. Walk the subtree and decrement.
//   The Euler-tour variant below avoids the recursive re-add entirely and is
//     usually the faster one to write: tin/tout give each subtree as a
//     contiguous range, so re-adding a light subtree is a flat loop.
//   Queries "on a path" are NOT this. This is subtrees only.
//   The root is called with keep = true, so when solve() returns the counters
//     still hold the WHOLE tree. Multi-test-case: clear cnt[] and the
//     aggregate yourself, or case 2 starts with case 1 still loaded.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   most frequent colour in each subtree              cnt[] + a running best
//   number of DISTINCT colours in each subtree        cnt[] + a `distinct`
//                                                     counter
//   sum of the colours appearing exactly k times       cnt-of-cnt array
//   for each subtree, how many pairs share a colour    += cnt[c] before adding
//   subtree queries offline, arbitrary aggregate       this, if add/remove are
//                                                     both O(1)
//   ... aggregate is a k-th / range count              segment tree merging
//                                                     instead
//   "count paths through v with property P"            centroid decomposition
//                                                     instead -- DS/centroid/
//   merge sets of children, answer per node            small_to_large() below
//
// NOT THIS:
//   answers needed for PATHS, not subtrees -> Mo's on tree, or HLD.
//   the tree changes -> nothing here survives; link-cut territory.
//   values need range queries inside the subtree -> Euler tour + segment tree
//     is simpler and O(n log n) with no cleverness.
// ============================================================================
int n;
vector<int> g[200005];
int col[200005], sz[200005], heavy[200005];
ll  ans[200005];

// --- the aggregate being maintained. Replace this block per problem. -------
int cnt[200005];             // cnt[c] = how many nodes of colour c are in
int distinct_;               // number of colours with cnt > 0

void add(int v)    { if (cnt[col[v]]++ == 0) distinct_++; }
void remove_(int v){ if (--cnt[col[v]] == 0) distinct_--; }
// ---------------------------------------------------------------------------

void calc_sz(int v, int p) {
    sz[v] = 1, heavy[v] = -1;
    for (int c : g[v]) {
        if (c == p) continue;
        calc_sz(c, v);
        sz[v] += sz[c];
        if (heavy[v] < 0 || sz[c] > sz[heavy[v]]) heavy[v] = c;
    }
}
// add every node of v's subtree, skipping the subtree rooted at `skip`
void add_sub(int v, int p, int skip) {
    add(v);
    for (int c : g[v]) if (c != p && c != skip) add_sub(c, v, skip);
}
void del_sub(int v, int p) {
    remove_(v);
    for (int c : g[v]) if (c != p) del_sub(c, v);
}

void dfs(int v, int p, bool keep) {
    for (int c : g[v])                              // 1. light children first
        if (c != p && c != heavy[v]) dfs(c, v, false);
    if (heavy[v] >= 0) dfs(heavy[v], v, true);      // 2. heavy child, kept

    add(v);                                         // 3. v and the light subs
    for (int c : g[v])
        if (c != p && c != heavy[v]) add_sub(c, v, -1);

    ans[v] = distinct_;                             // <- read it HERE

    if (!keep) del_sub(v, p);                       // 4. clean up if light
}

void solve(int root) { calc_sz(root, 0); dfs(root, 0, true); }

////////////////////////////////////////////////////////////////////////////////
// EULER-TOUR FORM -- same complexity, no recursive re-add, and the one to
// write under pressure. tin/tout flatten each subtree into a range of `ord`,
// so "add every node of this light subtree" is a flat for-loop.
////////////////////////////////////////////////////////////////////////////////
int tin[200005], tout[200005], ord[200005], timer_;

void flat(int v, int p) {
    tin[v] = timer_, ord[timer_++] = v;
    sz[v] = 1, heavy[v] = -1;
    for (int c : g[v]) {
        if (c == p) continue;
        flat(c, v);
        sz[v] += sz[c];
        if (heavy[v] < 0 || sz[c] > sz[heavy[v]]) heavy[v] = c;
    }
    tout[v] = timer_;
}
void dfs2(int v, int p, bool keep) {
    for (int c : g[v])
        if (c != p && c != heavy[v]) dfs2(c, v, false);
    if (heavy[v] >= 0) dfs2(heavy[v], v, true);

    for (int c : g[v])                              // light subtrees, flat
        if (c != p && c != heavy[v])
            for (int i = tin[c]; i < tout[c]; i++) add(ord[i]);
    add(v);

    ans[v] = distinct_;

    if (!keep)
        for (int i = tin[v]; i < tout[v]; i++) remove_(ord[i]);
}
void solve2(int root) { timer_ = 0; flat(root, 0); dfs2(root, 0, true); }

////////////////////////////////////////////////////////////////////////////////
// SMALL TO LARGE, the generic container version. Use when the aggregate is NOT
// undoable in O(1) and you genuinely need a set/map per node.
//
//   ALWAYS iterate the SMALLER container into the larger, then swap.
//   Backwards it is O(n^2) and looks identical.
//
// O(n log^2 n) with a set (log for the container, log for the depth).
////////////////////////////////////////////////////////////////////////////////
set<int> *S[200005];

void stl_dfs(int v, int p) {
    S[v] = new set<int>();
    S[v]->insert(col[v]);
    for (int c : g[v]) {
        if (c == p) continue;
        stl_dfs(c, v);
        if (S[c]->size() > S[v]->size()) swap(S[v], S[c]);   // <- the swap
        for (int x : *S[c]) S[v]->insert(x);
        S[c]->clear();
    }
    ans[v] = S[v]->size();
}
