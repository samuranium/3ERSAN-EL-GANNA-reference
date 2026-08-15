// ============================================================================
// MO'S ON A TREE -- path queries offline, O((n + q) sqrt n)
// ----------------------------------------------------------------------------
// Flatten the tree into an EULER SEQUENCE of length 2n: append u on entry and
// again on exit. Then a path query becomes a contiguous RANGE in that sequence,
// and ordinary Mo's runs on it unchanged.
//
// ############################################################################
// #  THE RULE THAT MAKES IT WORK: APPEARING TWICE MEANS ABSENT
// #
// #  A vertex inside the range twice (entry AND exit) is NOT on the path -- it
// #  is a side branch that the walk entered and left. A vertex appearing ONCE
// #  is on the path.
// #
// #  So add/remove is a TOGGLE: seeing a vertex flips whether it counts.
// #  That single line is the entire difference from array Mo's.
// ############################################################################
//
// ############################################################################
// #  THE LCA IS THE SPECIAL CASE, AND IT IS ASYMMETRIC
// #
// #  With tin[u] <= tin[v]:
// #    l = lca(u,v) == u   ->  range is [tin[u], tin[v]]          LCA included
// #    otherwise           ->  range is [tout[u], tin[v]]   and the LCA appears
// #                            ZERO times, so add it by hand before answering
// #                            and remove it after.
// #
// #  Forgetting the manual LCA is the classic bug: correct whenever one
// #  endpoint is an ancestor of the other, wrong otherwise.
// ############################################################################
//
// PITFALLS:
//   The Euler array has length 2n, so block size is sqrt(2n) and every bound
//     doubles. Sizing arrays by n instead of 2n is silent corruption.
//   Offline only -- all queries must be known before sorting.
//   add()/remove() must be O(1) amortised, exactly as in array Mo's.
//   Values usually need compressing into [0, n) first; see compress().
//   With updates as well as queries, this becomes Mo's with modifications
//     (three-dimensional ordering) -- a different, heavier algorithm.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   # distinct values on the path u..v      the canonical use
//   # values occurring exactly k times      same counter, different answer
//   most frequent value on the path         maintain a frequency-of-frequency
//   sum / xor over distinct values          same toggle, different accumulator
//   count pairs on the path with some       any statistic that supports O(1)
//     property                              insert and erase
//   subtree queries instead of path         much easier -- tin/tout is already
//                                           a contiguous range, use array Mo's
//                                           or a segment tree directly
//
// NOT THIS: if the statistic is decomposable (sum, max, gcd) use HLD or LCA
//   with prefix aggregates -- O(log n) per query and online. Mo's earns its
//   place only for statistics like "distinct count" that do not decompose.
// ============================================================================
// needs lca(), tin[], dep[] from DS/LCA/binary lifting.cpp
const int MN = 200'123;
int euler[2 * MN], first_[MN], last_[MN], etime;
int a[MN], comp[MN];                 // a[] = value at each vertex, compressed
bool inpath[MN];                     // is this vertex currently counted
int freq[MN], distinctCnt;

void euler_dfs(int u, int p) {
    first_[u] = etime, euler[etime++] = u;
    for (int v : g[u])
        if (v != p) euler_dfs(v, u);
    last_[u] = etime, euler[etime++] = u;
}
// TOGGLE -- see the box
void toggle(int u) {
    if (inpath[u]) {                 // remove
        if (--freq[a[u]] == 0) distinctCnt--;
    } else {                         // add
        if (freq[a[u]]++ == 0) distinctCnt++;
    }
    inpath[u] ^= 1;
}
struct Q {
    int l, r, lc, idx;               // lc = the LCA to add manually, or -1
};
int BS;                              // block size = sqrt(2n)
bool operator<(const Q &x, const Q &y) {
    int bx = x.l / BS, by = y.l / BS;
    if (bx != by) return bx < by;
    return (bx & 1) ? x.r > y.r : x.r < y.r;
}
// build one query for the path u..v
Q make_query(int u, int v, int idx) {
    if (first_[u] > first_[v]) swap(u, v);
    int l = lca(u, v);
    if (l == u) return {first_[u], first_[v], -1, idx};
    return {last_[u], first_[v], l, idx};          // LCA appears zero times
}
void run(vector<Q> &qs, vector<int> &ans) {
    BS = max(1, (int)sqrt((double)etime));
    sort(qs.begin(), qs.end());
    int L = 0, R = -1;
    for (auto &q : qs) {
        while (R < q.r) toggle(euler[++R]);
        while (L > q.l) toggle(euler[--L]);
        while (R > q.r) toggle(euler[R--]);
        while (L < q.l) toggle(euler[L++]);
        if (q.lc != -1) toggle(q.lc);              // add the LCA by hand
        ans[q.idx] = distinctCnt;
        if (q.lc != -1) toggle(q.lc);              // and take it back out
    }
}
