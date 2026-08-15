// ============================================================================
// DSU FOR MARKING -- two different tricks that both get called "DSU marking"
// ----------------------------------------------------------------------------
//   [A] DSU AS A NEXT-UNMARKED POINTER   -- the one people mean, and the one
//                                           that is not obviously a DSU at all
//   [B] MARKS CARRIED ON THE COMPONENT   -- ordinary DSU with a payload
//
// ============================================================================
// [A] NEXT-UNMARKED POINTER  ("painting", "skip what is done")
// ----------------------------------------------------------------------------
// WHEN YOU SEE: an operation that touches a RANGE, but each element may only be
//   touched ONCE overall. Colour each cell in [l,r] that is still white. Assign
//   each request the first free seat at or after x. Fill each empty slot.
//   Naively that is O(n) per operation and O(n*q) total; with this it is O(n a)
//   TOTAL, because every element is consumed exactly once for the whole run.
//
// THE IDEA: nxt[i] = the smallest index >= i that is still unmarked. Marking i
//   means "i now forwards to i+1", and path compression collapses long runs of
//   consumed cells into one hop. The DSU is not grouping anything -- it is a
//   pointer that only ever moves right.
//
// ############################################################################
// #  SIZE IT n + 2 AND SEED THE SENTINEL
// #
// #      for (i = 0; i <= n + 1; i++) nxt[i] = i;
// #
// #  find() on the last element must be able to walk to n+1 and stop. If the
// #  array ends at n, marking the final element runs off the end -- and it is
// #  an out-of-bounds READ, so it usually does not crash, it just returns
// #  garbage and marks a cell that does not exist.
// #  find(i) == n + 1 is the "nothing free from i onwards" answer. Check it.
// ############################################################################
//
// SUBPROBLEMS:
//   paint [l,r], each cell painted once     see paint() -- the loop is the API
//   first free slot >= x, then take it      i = find(x); if (i <= n) mark(i);
//   assign seats / rooms / parking          same
//   fill each zero with a value, once       same
//   "apply q range ops, each cell final     same, iterate with find()
//     after its first hit"
//   next unmarked to the LEFT               a second array walking the other
//                                           way, prv[i] = largest <= i unmarked
//   erase from a sorted range, offline      same as marking
//
// NOT THIS: if a cell can be re-marked and UNMARKED, a DSU cannot go backwards.
//   Use a set<int> of free positions (O(log n), simpler to reason about), or a
//   segment tree with a descend (DS/segment tree/lazy.cpp [1]).
//
// ============================================================================
// [B] MARKS ON THE COMPONENT
// ----------------------------------------------------------------------------
// WHEN YOU SEE: "is there a special vertex in my component", "how many marked
//   in my group", "the largest / smallest id still active here". Merging two
//   components merges their payloads.
//
// The payload rule: whatever you store must be MERGEABLE in O(1) and must live
//   only on the ROOT. Reading it anywhere but at find(x) gives a stale value.
//
//   count of marked          cnt[a] += cnt[b]
//   any marked at all        any[a] |= any[b]
//   min / max member id      lo[a] = min(lo[a], lo[b])
//   size                     sz[a] += sz[b]
//   sum of weights           sum[a] += sum[b]
//
// PITFALL: marking a single element is cnt[find(x)]++, NOT cnt[x]++. Writing to
//   a non-root is the standard bug here and it survives until the first union.
// ============================================================================
const int N = 200'123;

// ---------------- [A] next-unmarked pointer ----------------
int nxt[N + 2];

void init_marks(int n) {
    for (int i = 0; i <= n + 1; i++) nxt[i] = i;   // n+1 = sentinel
}
int find(int x) { return nxt[x] == x ? x : nxt[x] = find(nxt[x]); }
void mark(int x) { nxt[x] = x + 1; }              // x is consumed

// paint every still-unmarked cell in [l, r]. Amortised O(a) per cell, once.
void paint(int l, int r, int colour, int *out) {
    for (int i = find(l); i <= r; i = find(i))
        out[i] = colour, mark(i);
}

// ---------------- [B] marks carried on the component ----------------
int par[N], sz_[N], cnt[N];

void init_dsu(int n) {
    for (int i = 0; i <= n; i++) par[i] = i, sz_[i] = 1, cnt[i] = 0;
}
int root(int x) { return par[x] == x ? x : par[x] = root(par[x]); }
void set_mark(int x) { cnt[root(x)]++; }          // NOT cnt[x]++
int marks_here(int x) { return cnt[root(x)]; }
void unite(int a, int b) {
    a = root(a), b = root(b);
    if (a == b) return;
    if (sz_[a] < sz_[b]) swap(a, b);
    par[b] = a, sz_[a] += sz_[b], cnt[a] += cnt[b];
}
