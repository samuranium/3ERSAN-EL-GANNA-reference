// ============================================================================
// PARALLEL BINARY SEARCH -- "when does each query first become satisfiable"
// ----------------------------------------------------------------------------
// Every query wants to binary search the SAME time axis: "after how many
// events does my condition hold?" Doing them one at a time is q separate
// searches, each replaying the events: O(q * n * log n).
//
// Instead binary search ALL queries at once. At each of the log levels, group
// the queries by their current midpoint, replay the events ONCE in order, and
// answer every query as its midpoint is passed. Total O((n + q) log n * cost).
//
//     lo[i], hi[i]   the surviving interval for query i
//     bucket[t]      the queries whose midpoint is t, this round
//
// ############################################################################
// #  THE EVENTS ARE REPLAYED FROM SCRATCH ONCE PER LEVEL, IN ORDER
// #
// #  Each round: reset the structure, then walk t = 1..n applying event t and
// #  answering every query in bucket[t]. You are NOT allowed to jump the
// #  structure to an arbitrary time -- that is the whole reason this is
// #  cheaper than q independent searches. Resetting is O(n) with a plain
// #  array; if the reset is expensive, use a rollback structure instead.
// ############################################################################
//
// ############################################################################
// #  A QUERY THAT IS NEVER SATISFIED MUST END WITH lo = n+1
// #
// #  Initialise hi = n + 1 (one past the last event) and treat lo > n as
// #  "impossible". Initialising hi = n makes an unsatisfiable query report the
// #  last event as its answer -- a wrong answer that is indistinguishable
// #  from a legitimate one.
// ############################################################################
//
// PITFALLS:
//   The condition must be MONOTONE in time: once true it stays true. If an
//     event can un-satisfy a query, binary search is invalid entirely.
//   Bucketing must be rebuilt each round; a stale bucket answers a query at
//     the wrong time.
//   The loop terminates when every lo == hi. Guard with a round counter of
//     ceil(log2(n+2)) rather than "until nothing changes", which can loop.
//   With a DSU inside, use union by size with NO path compression only if you
//     need rollback; a full reset per round is usually simpler and faster.
//   Answering "the earliest event" and "the value at that moment" are two
//     different reads -- capture what you need while you are at time t.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   "after which event do u and v become connected"    the canonical use, DSU
//   "the smallest weight w making u,v connected"        sort edges by w, this;
//                                                       or Kruskal
//                                                       reconstruction tree --
//                                                       graph/MST/kruskal_tree.cpp
//   "when does this cell first get filled"              events are fills
//   "when does the k-th ball land in bin i"             events are throws, the
//                                                       structure is a BIT
//   "the first prefix whose sum exceeds x", per query    BIT + this, or a
//                                                       segment tree descend
//   "the first time this region receives k units"       BIT over positions
//   each query needs its own binary search over a       exactly this
//     SHARED sequence of updates
//
// NOT THIS:
//   the condition is not monotone in time -> binary search is wrong, not just
//     slow.
//   there is only one query               -> just binary search it directly.
//   an online answer is required          -> this is offline by construction.
//   the structure supports rollback cheaply and the events form a tree ->
//     segment tree on time + rollback DSU is a better fit,
//     graph/dsu/dsu_rollback.cpp.
// ============================================================================
const int MAXN = 200005;

int n, q;                         // n events (1-indexed), q queries

// ---- the example: edges arrive one per unit time; each query asks when u,v
//      first become connected. Swap this block for the real problem.
int eu[MAXN], ev[MAXN];           // event t adds the edge (eu[t], ev[t])
int qu[MAXN], qv[MAXN];           // query i asks about qu[i], qv[i]

int dsu[MAXN], sz_[MAXN];
void dsu_reset(int m) { for (int i = 0; i <= m; i++) dsu[i] = i, sz_[i] = 1; }
int find(int x) { return dsu[x] == x ? x : dsu[x] = find(dsu[x]); }
void unite(int a, int b) {
    a = find(a), b = find(b);
    if (a == b) return;
    if (sz_[a] < sz_[b]) swap(a, b);
    dsu[b] = a, sz_[a] += sz_[b];
}
bool satisfied(int i) { return find(qu[i]) == find(qv[i]); }
// ---------------------------------------------------------------------------

int lo[MAXN], hi[MAXN];
vector<int> bucket_[MAXN];

// returns, per query, the smallest t with the condition true, or n+1 if never.
// t = 0 means "already true before any event" -- that is why lo starts at 0.
void parallel_binary_search(int vertices) {
    for (int i = 0; i < q; i++) lo[i] = 0, hi[i] = n + 1;   // n+1 = "never"
    int rounds = 1;
    while ((1 << rounds) < n + 2) rounds++;
    for (int it = 0; it <= rounds; it++) {
        for (int t = 0; t <= n; t++) bucket_[t].clear();
        bool any = false;
        for (int i = 0; i < q; i++)
            if (lo[i] < hi[i]) {
                bucket_[lo[i] + hi[i] >> 1].push_back(i);   // mid is in [0,n]
                any = true;
            }
        if (!any) break;
        dsu_reset(vertices);                                // full reset
        for (int i : bucket_[0])                            // BEFORE any event
            if (satisfied(i)) hi[i] = 0;
            else lo[i] = 1;
        for (int t = 1; t <= n; t++) {
            unite(eu[t], ev[t]);                            // apply event t
            for (int i : bucket_[t])                        // answer at time t
                if (satisfied(i)) hi[i] = t;
                else lo[i] = t + 1;
        }
    }
    // lo[i] IS the answer: the first t at which query i holds.
    // lo[i] == 0   -> already true before any event
    // lo[i] == n+1 -> never becomes true
}
