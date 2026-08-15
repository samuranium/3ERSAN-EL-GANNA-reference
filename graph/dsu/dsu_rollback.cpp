// ============================================================================
// DSU WITH ROLLBACK -- union by size only, undo in O(1), find in O(log n)
// ----------------------------------------------------------------------------
// ############################################################################
// #  NO PATH COMPRESSION. THAT IS THE WHOLE DESIGN.
// #
// #  Path compression rewrites parents of nodes you never touched, so there is
// #  no bounded record of what changed and an undo is impossible. Union by
// #  size alone still gives O(log n) depth, which is the price of undo.
// #
// #  find() is therefore a LOOP, not a recursive one-liner, and it must not
// #  assign anything. If you paste the usual `p[x] = find(p[x])` in here, the
// #  structure keeps working and rollback silently corrupts it.
// ############################################################################
//
// Each successful unite pushes ONE record. rollback() undoes exactly one
// unite, so the caller counts: save the size of the history, do work, roll
// back until the history is that size again.
//
// PITFALLS:
//   A unite that finds a == b pushes NOTHING. Rolling back a fixed number of
//     times rather than to a saved checkpoint is therefore wrong. Always
//     snapshot hist.size().
//   comps is maintained here because "number of components" is what most
//     rollback problems actually query.
//   No path compression means find is O(log n), not near-O(1). At 1e5 unions
//     inside a segment-tree-on-time sweep that log matters -- budget it.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS -- rollback exists for these three shapes
// ----------------------------------------------------------------------------
// [1] OFFLINE DYNAMIC CONNECTIVITY. Edges appear and disappear over time.
//     Put each edge on the segment tree over the TIME axis, covering the
//     interval it is alive. DFS the segment tree: on entry unite the edges
//     stored at that node, on exit roll them back. At a leaf you have exactly
//     the edges alive at that moment. O(q log q log n).
//     This is the canonical use and the reason the file exists.
//
// [2] DIVIDE AND CONQUER ON EDGES / QUERIES. Same idea without an explicit
//     time axis: recurse, unite the edges common to a half, roll back on
//     return.
//
// [3] MO WITH DSU. Mo's ordering needs deletions, which a plain DSU cannot do.
//     Rollback gives you the "add, answer, undo" pattern instead.
//
//   Also: "is the graph connected if I remove edge e, for every e" -- run [1]
//   with each edge alive on the complement interval.
//   And: minimum spanning tree under edge updates, offline.
//
// NOT THIS: if edges are only ever ADDED, a normal DSU with path compression
//   is faster and simpler. If you need ONLINE deletions, this cannot help --
//   that is link-cut trees or Euler tour trees.
// ============================================================================
struct DSURollback {
    vector<int> p, sz;
    vector<pair<int, int>> hist;      // (child root, parent root) per real union
    int comps;

    DSURollback(int n) : p(n), sz(n, 1), comps(n) {
        iota(p.begin(), p.end(), 0);
    }
    int find(int x) const {           // NO compression -- see the box
        while (p[x] != x) x = p[x];
        return x;
    }
    bool unite(int a, int b) {
        a = find(a), b = find(b);
        if (a == b) return false;     // pushes nothing: snapshot, do not count
        if (sz[a] < sz[b]) swap(a, b);
        hist.push_back({b, a});
        p[b] = a, sz[a] += sz[b], comps--;
        return true;
    }
    void rollback() {                 // undo exactly one successful unite
        if (hist.empty()) return;
        auto [b, a] = hist.back();
        hist.pop_back();
        p[b] = b, sz[a] -= sz[b], comps++;
    }
    int snapshot() const { return hist.size(); }
    void rollback_to(int mark) { while ((int)hist.size() > mark) rollback(); }
    bool same(int a, int b) const { return find(a) == find(b); }
    int size(int a) const { return sz[find(a)]; }
};
