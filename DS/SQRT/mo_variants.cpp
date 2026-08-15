// ============================================================================
// MO'S VARIANTS -- rollback (no remove), with updates (3D), and Hilbert order
// ----------------------------------------------------------------------------
// Plain Mo's is DS/SQRT/mo.cpp. Three situations break it, and each has a
// standard fix.
//
//   [1] remove() is hard or impossible    -> ROLLBACK MO'S (add-only + undo)
//   [2] there are point UPDATES too       -> 3D MO, block n^(2/3), O(n^(5/3))
//   [3] correct but too slow              -> HILBERT ORDER, or block n/sqrt(q)
//
// ############################################################################
// #  [1] ROLLBACK MO'S NEVER CALLS remove(). THAT IS THE ENTIRE POINT.
// #
// #  Some aggregates can be added to but not removed from: maximum, "the
// #  longest run", "the largest value times its count". Undoing them needs
// #  the whole history.
// #
// #  The fix: group queries by L's BLOCK. Within a block, R only increases.
// #      - queries entirely inside one block  -> brute force them, O(sqrt n)
// #      - otherwise: keep a persistent right part (R grows monotonically,
// #        never rewound), then extend LEFT from the block's right edge,
// #        answer, and ROLL BACK the left extension by restoring a SNAPSHOT.
// #
// #  The snapshot must be O(1) to take and restore. Save the scalar answer
// #  and undo the counter writes -- do NOT memcpy the counter array.
// ############################################################################
//
// ############################################################################
// #  [2] 3D MO'S BLOCK SIZE IS n^(2/3), NOT sqrt(n)
// #
// #  With three keys (L block, R block, time) the optimum is B = n^(2/3),
// #  giving O(n^(5/3)). Using sqrt(n) blocks makes it O(n^2) -- correct, and
// #  far too slow. The time pointer must be moved BEFORE or AFTER the l/r
// #  pointers consistently, and applying an update whose position is inside
// #  [l,r] has to remove the old value and add the new one.
// ############################################################################
//
// PITFALLS:
//   Rollback Mo's: reset the structure completely at the start of each L
//     block, and handle the "query fits in one block" case separately or the
//     left extension goes past R.
//   Rollback: `save()` before extending left, `rollback()` after answering --
//     every iteration, even when the extension was empty.
//   3D Mo: `t` counts how many updates precede the query, so read the queries
//     and updates in input order and stamp them.
//   3D Mo: an update at position p only affects the answer if l <= p <= r at
//     the moment it is applied. Always swap the stored old/new value so the
//     operation is its own inverse.
//   Hilbert order replaces the comparator only. Everything else is unchanged,
//     and it removes the odd/even R-direction hack.
//   Mo's of any flavour is OFFLINE. Forced-online kills all of it.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   range queries, no updates, removable aggregate   plain Mo's, DS/SQRT/mo.cpp
//   ... aggregate cannot be removed (max, longest    rollback Mo's [1]
//       run, max value*count)
//   ... plus point assignments                       3D Mo's [2]
//   ... on tree paths                                DS/SQRT/mo_on_tree.cpp
//   q much smaller than n                            block = n/sqrt(q), not
//                                                    sqrt(n)
//   correct but TLE by a constant factor             Hilbert order [3]
//   "distinct values in a range", offline            simpler: sort by r + a
//                                                    BIT, O((n+q) log n)
//   range mode                                       block-pair table, not
//                                                    Mo's
//
// NOT THIS:
//   the queries arrive online   -> persistent / wavelet / dynamic segtree.
//   the aggregate is mergeable  -> a segment tree is O(log n), not O(sqrt n).
// ============================================================================
const int N = 200005;

int n, q, a[N];
ll cur;                              // the aggregate being maintained
int frq[N];

void add(int i) { cur += ++frq[a[i]]; }        // example: sum of frq^2 deltas
void rem(int i) { cur -= frq[a[i]]--; }

////////////////////////////////////////////////////////////////////////////////
// [1] ROLLBACK MO'S -- add-only. rem() is never called.
////////////////////////////////////////////////////////////////////////////////
int B1;                              // block size, ~sqrt(n)

struct Q1 { int l, r, i; };
ll out1[N];

// snapshot/restore: record the writes, undo them. O(1) per touched element.
ll saved_cur;
vector<int> touched;

void add_tracked(int i) { touched.push_back(a[i]); cur += ++frq[a[i]]; }
void save()    { saved_cur = cur, touched.clear(); }
void rollback(){ for (int v : touched) cur -= frq[v]--; cur = saved_cur; touched.clear(); }

void mo_rollback(vector<Q1> &qq) {
    B1 = max(1, (int)sqrt((double)n));
    sort(qq.begin(), qq.end(), [](const Q1 &x, const Q1 &y) {
        int bx = x.l / B1, by = y.l / B1;
        if (bx != by) return bx < by;
        return x.r < y.r;                       // R only increases in a block
    });
    int i = 0;
    while (i < (int)qq.size()) {
        int b = qq[i].l / B1;
        int bend = min(n, (b + 1) * B1);        // one past the block's last idx
        // reset the structure for this block
        for (int j = 0; j < N; j++) frq[j] = 0;
        cur = 0;
        int r = bend - 1;                        // persistent right pointer
        while (i < (int)qq.size() && qq[i].l / B1 == b) {
            if (qq[i].r < bend) {                // fits inside the block: brute
                ll keep = cur;
                save();
                for (int j = qq[i].l; j <= qq[i].r; j++) add_tracked(j);
                out1[qq[i].i] = cur;
                rollback();
                cur = keep;
            } else {
                while (r < qq[i].r) add(++r);    // R grows, never rewound
                save();
                for (int j = bend - 1; j >= qq[i].l; j--) add_tracked(j);
                out1[qq[i].i] = cur;
                rollback();                       // undo ONLY the left part
            }
            i++;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// [2] MO'S WITH UPDATES (3D). Block = n^(2/3).
//
// Queries carry `t` = how many updates happened before them. Updates are
// (position, new value, old value) and applying one twice is the identity.
////////////////////////////////////////////////////////////////////////////////
struct Q3 { int l, r, t, i; };
struct Upd { int p, nv, ov; };
ll out3[N];
int B3;

void apply_upd(Upd &u, int l, int r) {
    if (l <= u.p && u.p <= r) {                 // only matters if it is inside
        rem(u.p);
        a[u.p] = u.nv;
        add(u.p);
    } else a[u.p] = u.nv;
    swap(u.nv, u.ov);                           // self-inverse
}

void mo_updates(vector<Q3> &qq, vector<Upd> &up) {
    B3 = max(1, (int)pow((double)n, 2.0 / 3.0));
    sort(qq.begin(), qq.end(), [](const Q3 &x, const Q3 &y) {
        int bx = x.l / B3, by = y.l / B3;
        if (bx != by) return bx < by;
        int rx = x.r / B3, ry = y.r / B3;
        if (rx != ry) return (bx & 1) ? rx > ry : rx < ry;
        return (rx & 1) ? x.t > y.t : x.t < y.t;
    });
    int l = 0, r = -1, t = 0;
    for (auto &Q : qq) {
        while (t < Q.t) apply_upd(up[t], l, r), t++;
        while (t > Q.t) t--, apply_upd(up[t], l, r);
        while (r < Q.r) add(++r);
        while (l > Q.l) add(--l);
        while (r > Q.r) rem(r--);
        while (l < Q.l) rem(l++);
        out3[Q.i] = cur;
    }
}

////////////////////////////////////////////////////////////////////////////////
// [3] HILBERT ORDER -- a drop-in comparator. Replace the sort key, change
// nothing else. Removes the odd/even R-direction hack and is measurably
// faster on large inputs.
//
// `pw` must satisfy 2^pw > n.
////////////////////////////////////////////////////////////////////////////////
ll hilbert(int x, int y, int pw = 21) {
    ll rx, ry, d = 0;
    for (ll s = 1LL << (pw - 1); s > 0; s >>= 1) {
        rx = (x & s) > 0, ry = (y & s) > 0;
        d += s * s * ((3 * rx) ^ ry);
        // rotate
        if (ry == 0) {
            if (rx == 1) x = s - 1 - x, y = s - 1 - y;
            swap(x, y);
        }
    }
    return d;
}
// usage: sort(qq.begin(), qq.end(), [](auto &x, auto &y){
//            return hilbert(x.l, x.r) < hilbert(y.l, y.r); });
