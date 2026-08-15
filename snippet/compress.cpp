// ============================================================================
// COORDINATE COMPRESSION -- map arbitrary values into [0, k)
// ----------------------------------------------------------------------------
// Four lines, used constantly. Written down because the two variants below are
// easy to confuse and the wrong one is a silent off-by-one.
//
// ############################################################################
// #  RANK vs INDEX -- decide which you need
// #
// #   DENSE RANK (this file): equal values get the SAME id, ids are 0..k-1
// #                           with no gaps. Use for "value -> array slot".
// #   ORDER INDEX:            every element gets a DISTINCT id (ties broken by
// #                           position). Use for permutation-style problems.
// #
// #  Using dense rank where you needed distinct ids collapses duplicates and
// #  silently loses elements. Using distinct ids as array slots wastes k = n
// #  slots and breaks "count values equal to x".
// ############################################################################
//
// PITFALLS:
//   Compress ALL values that will ever be looked up -- query bounds too, not
//     just the array. A lookup of an uncompressed value returns a neighbour's
//     id with no error.
//   For a range query [l, r] on compressed values, compress l and r with
//     lower_bound / upper_bound respectively, not both the same way.
//   If the values are already small (< 1e6) do not compress -- indexing
//     directly is faster and shorter.
//
// SUBPROBLEMS:
//   values up to 1e9, need an array indexed by value   this
//   Fenwick / segment tree over values                 this first
//   count distinct values                              k, the compressed size
//   "how many distinct values ≤ x"                     upper_bound on `vals`
//   offline queries touching arbitrary points          compress array + queries
//   2D: compress x and y independently                 two of these
// ============================================================================

// dense rank: returns the sorted unique values; a[] is rewritten to ids 0..k-1
template <class T>
vector<T> compress(vector<T> &a) {
    vector<T> vals = a;
    sort(vals.begin(), vals.end());
    vals.erase(unique(vals.begin(), vals.end()), vals.end());
    for (auto &x : a)
        x = lower_bound(vals.begin(), vals.end(), x) - vals.begin();
    return vals;                       // vals[id] recovers the original value
}
// look a value up afterwards; -1 if it was never compressed
template <class T>
int id_of(const vector<T> &vals, T x) {
    auto it = lower_bound(vals.begin(), vals.end(), x);
    return (it != vals.end() && *it == x) ? it - vals.begin() : -1;
}
// how many compressed values are < x  /  <= x  -- for range bounds
template <class T>
int lt(const vector<T> &vals, T x) {
    return lower_bound(vals.begin(), vals.end(), x) - vals.begin();
}
template <class T>
int le(const vector<T> &vals, T x) {
    return upper_bound(vals.begin(), vals.end(), x) - vals.begin();
}
// ORDER INDEX: every element a distinct id, ties broken by original position
vector<int> order_index(const vector<ll> &a) {
    int n = a.size();
    vector<int> ord(n);
    iota(ord.begin(), ord.end(), 0);
    sort(ord.begin(), ord.end(),
         [&](int x, int y) { return a[x] != a[y] ? a[x] < a[y] : x < y; });
    vector<int> id(n);
    for (int i = 0; i < n; i++) id[ord[i]] = i;
    return id;
}
