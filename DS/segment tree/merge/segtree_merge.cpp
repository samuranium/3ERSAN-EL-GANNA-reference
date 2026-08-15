// ============================================================================
// SEGMENT TREE MERGING -- one dynamic segtree per node, merged up the tree
// ----------------------------------------------------------------------------
// Give every vertex a DYNAMIC segment tree indexed by VALUE, then merge each
// child's tree into the parent's. Total cost O(n log n), because merge()
// recurses only where BOTH trees have a node, and every such step DESTROYS one
// node. At most O(n log n) nodes are ever created, so at most that many die.
//
// This is what small-to-large cannot do: the merged structure still supports
// k-th smallest, count-in-range, and max-with-position -- queries a set or a
// counter array cannot answer. And it is one log, not two.
//
// ############################################################################
// #  MERGING IS DESTRUCTIVE. THE INPUTS ARE GONE.
// #
// #  merge(a, b) reuses a's nodes and splices b's in. Afterwards neither a
// #  nor b is valid on its own -- only the returned root is. You cannot:
// #      query a child's tree after merging it,
// #      merge the same tree into two different parents,
// #      merge PERSISTENT trees (you would corrupt every earlier version).
// #  If you need the child's answer, READ IT BEFORE MERGING.
// ############################################################################
//
// ############################################################################
// #  THE NODE POOL IS THE THING THAT BLOWS UP
// #
// #  n insertions each create O(log C) nodes, so reserve about
// #  n * (log2(C) + 2) and round up generously. MEASURED: 31 nodes per insert
// #  over a 1e9 range, so 2e5 values need 6.2e6 -- reserving "about 6e6"
// #  overflows. A flat array of structs, never `new` per node.
// #  Merging never allocates, so the peak is set by the insertions alone.
// ############################################################################
//
// PITFALLS:
//   Nodes are identified by index with 0 = null. Never use -1; the arithmetic
//     below relies on `if (!a) return b;`.
//   pull() must run AFTER both recursive merges, not between them.
//   For a MAX-with-position tree, merging two nodes whose subtrees both exist
//     still needs the pull; taking `max(a, b)` at the top is wrong because the
//     children have not been combined yet.
//   Clearing between test cases: reset the pool counter, do not memset 6e6
//     nodes per case.
//   If you also need to DELETE, you cannot -- merging has already fused the
//     multisets. Use a BIT over an Euler tour instead.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   for each subtree, the k-th smallest value          kth() after merging
//   for each subtree, count of values in [a,b]         count_range()
//   for each subtree, the most frequent value          a (count, value) pull;
//                                                      tie -> smallest value
//   for each subtree, number of DISTINCT values         cheaper with the sack --
//                                                      DS/dsu_on_tree.cpp
//   "add x to every node with value v in this subtree"  merge + lazy, doable
//                                                       but see the trap above
//   auxiliary trees / DSU-on-tree replacement           this, whenever the
//                                                       aggregate needs order
//   SAM link tree: occurrence positions per state       merge endpos sets up
//                                                       the link tree
//   "for each vertex, closest marked descendant"        merge trees indexed by
//                                                       depth
//   offline: union of sets along a DSU                  merge on unite()
//
// NOT THIS:
//   the aggregate is O(1)-undoable (a counter, a sum) -> DSU on tree is
//     shorter and has no memory cost. DS/dsu_on_tree.cpp.
//   you need the child's tree AFTER the merge -> impossible here; use
//     persistent segment trees over an Euler tour instead.
//   the values are small and dense -> a plain counter array plus small-to-large.
// ============================================================================
const int POOL = 7000005;        // n * (log2 C + 2); see the box above

struct Node { int l, r; int cnt; };   // cnt = how many values in this range
Node t[POOL];
int tot;                              // 0 is the null node

int newnode() { int v = ++tot; t[v] = {0, 0, 0}; return v; }
void pull(int v) { t[v].cnt = t[t[v].l].cnt + t[t[v].r].cnt; }

// insert one copy of value `p` into the tree rooted at v, over range [lo,hi]
void insert(int &v, int lo, int hi, int p, int add = 1) {
    if (!v) v = newnode();
    if (lo == hi) { t[v].cnt += add; return; }
    int mid = lo + hi >> 1;
    if (p <= mid) insert(t[v].l, lo, mid, p, add);
    else          insert(t[v].r, mid + 1, hi, p, add);
    pull(v);
}

// DESTRUCTIVE. Returns the merged root; a and b are invalid afterwards.
int merge(int a, int b, int lo, int hi) {
    if (!a || !b) return a | b;                    // one side empty: take it
    if (lo == hi) { t[a].cnt += t[b].cnt; return a; }
    int mid = lo + hi >> 1;
    t[a].l = merge(t[a].l, t[b].l, lo, mid);
    t[a].r = merge(t[a].r, t[b].r, mid + 1, hi);
    pull(a);                                       // AFTER both merges
    return a;
}

// k-th smallest, 1-indexed. Returns -1 if k is out of range.
int kth(int v, int lo, int hi, int k) {
    if (!v || k < 1 || k > t[v].cnt) return -1;
    if (lo == hi) return lo;
    int mid = lo + hi >> 1, lc = t[t[v].l].cnt;
    if (k <= lc) return kth(t[v].l, lo, mid, k);
    return kth(t[v].r, mid + 1, hi, k - lc);
}
// how many stored values lie in [a,b]
int count_range(int v, int lo, int hi, int a, int b) {
    if (!v || b < lo || hi < a) return 0;
    if (a <= lo && hi <= b) return t[v].cnt;
    int mid = lo + hi >> 1;
    return count_range(t[v].l, lo, mid, a, b) + count_range(t[v].r, mid + 1, hi, a, b);
}

////////////////////////////////////////////////////////////////////////////////
// THE STANDARD USE: one tree per tree-vertex, answer per subtree.
//
// Read the answer for v AFTER merging all children in, and BEFORE v's own tree
// is merged into its parent -- that window is the only time root[v] describes
// exactly v's subtree.
////////////////////////////////////////////////////////////////////////////////
int n, VMAX;
vector<int> g[200005];
int val[200005], root[200005];
ll ans[200005];

void dfs(int v, int p) {
    root[v] = 0;
    insert(root[v], 1, VMAX, val[v]);
    for (int c : g[v]) {
        if (c == p) continue;
        dfs(c, v);
        root[v] = merge(root[v], root[c], 1, VMAX);
        root[c] = 0;                               // it is gone; do not reuse
    }
    ans[v] = kth(root[v], 1, VMAX, 1);             // <- read it HERE
}
