// ============================================================================
// 2D FENWICK, SPARSE / OFFLINE -- memory O(U log n), U = number of updates
// ----------------------------------------------------------------------------
// The dense fenwick_2d.cpp costs n*m cells. When the coordinate range is huge
// but the POINTS are few, keep a 1D Fenwick over x where each node stores only
// the y-values that actually reach it, compressed.
//
// ############################################################################
// #  THREE PHASES, IN ORDER. THIS IS NOT OPTIONAL.
// #
// #      1.  fake(x, y)   for EVERY update you will ever do
// #      2.  init()       sorts and compresses, allocates
// #      3.  upd/query    the real work
// #
// #  Phase 1 does not add anything -- it only reserves the y coordinate. An
// #  upd() at a (x, y) that was never faked writes into the wrong compressed
// #  slot and corrupts unrelated queries. It does not crash and it does not
// #  assert; you just get wrong answers.
// #  This is why it is called OFFLINE: you must know all update coordinates up
// #  front. Query coordinates are unrestricted.
// ############################################################################
//
// 1-INDEXED in x. y can be any int, including negative -- it is compressed.
//
// COMPLEXITY: build O(U log n log U). Each upd / query O(log n log U).
//   Memory O(U log n) -- every update reserves its y in log n nodes.
//
// PITFALLS:
//   pre(x, y) counts y' <= y, so it uses upper_bound, while upd() needs the
//     exact slot and uses lower_bound. Mixing them up is off by one row.
//   Query x beyond n is fine (the loop just stops); query x of 0 returns 0.
//   Coordinates that only ever appear in QUERIES need no fake() -- upper_bound
//     handles them.
//
// SUBPROBLEMS:
//   count points in a rectangle, offline      this
//   k-th smallest in a 2D region              this + binary search, or a
//                                             merge-sort tree
//   inversions with a second key              sort by one key, count by the other
//   "how many earlier points dominate me"     classic sweep + this
//   rectangle sum with sparse updates         this, weights instead of 1
//   IF updates arrive online                  BIT of sorted vectors is out;
//                                             use a BIT of order-statistic
//                                             trees, or sqrt decomposition
// ============================================================================
struct FT2D {
    int n;
    vector<vector<int>> ys;     // ys[i] = compressed y-values living in node i
    vector<vector<ll>> ft;      // ft[i] = 1D Fenwick over ys[i]

    FT2D(int n) : n(n), ys(n + 1), ft(n + 1) {}

    // PHASE 1 -- reserve, do not add
    void fake(int x, int y) {
        for (; x <= n; x += x & -x) ys[x].push_back(y);
    }
    // PHASE 2 -- call once, after every fake()
    void init() {
        for (int i = 1; i <= n; i++) {
            sort(ys[i].begin(), ys[i].end());
            ys[i].erase(unique(ys[i].begin(), ys[i].end()), ys[i].end());
            ft[i].assign(ys[i].size() + 1, 0);
        }
    }
    // PHASE 3 -- (x, y) must have been faked
    void upd(int x, int y, ll v) {
        for (; x <= n; x += x & -x) {
            int j = lower_bound(ys[x].begin(), ys[x].end(), y) - ys[x].begin() + 1;
            for (int k = j; k < (int)ft[x].size(); k += k & -k) ft[x][k] += v;
        }
    }
    ll pre(int x, int y) {      // sum over x' <= x and y' <= y
        ll s = 0;
        for (; x > 0; x -= x & -x) {
            int j = upper_bound(ys[x].begin(), ys[x].end(), y) - ys[x].begin();
            for (int k = j; k > 0; k -= k & -k) s += ft[x][k];
        }
        return s;
    }
    ll query(int x1, int y1, int x2, int y2) {   // inclusive rectangle
        return pre(x2, y2) - pre(x1 - 1, y2) - pre(x2, y1 - 1) + pre(x1 - 1, y1 - 1);
    }
};
