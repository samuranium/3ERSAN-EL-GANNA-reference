// ============================================================================
// WAVELET TREE -- range k-th smallest / rank / count, O(log V) per query
// ----------------------------------------------------------------------------
// A segment tree over the VALUE range instead of the index range. At each node
// the elements whose value falls in that node's half are kept, in their
// original relative order, and b[] records how many of the first i elements
// went LEFT. That prefix count is what lets a query descend: a range [l, r] at
// this node maps to a range at each child in O(1).
//
// ############################################################################
// #  IT BUILDS BY DESTROYING THE ARRAY
// #
// #  stable_partition rearranges the input in place, level by level. Pass a
// #  COPY unless you are done with the original. The relative order within
// #  each half must be preserved -- std::partition (unstable) silently breaks
// #  every query.
// ############################################################################
//
// ############################################################################
// #  WHEN TO PREFER THE PERSISTENT SEGMENT TREE INSTEAD
// #
// #  Both answer range k-th smallest in O(log V). Choose by what else you need:
// #    wavelet          smaller memory, no coordinate compression needed if
// #                     values are already small; also gives rank/count/quantile
// #                     naturally. STATIC only.
// #    persistent segtree  handles "k-th smallest in a version", and composes
// #                     with other persistent structures.
// #  Neither supports updates. If the array changes, you want a merge-sort
// #  tree with fractional cascading, or sqrt decomposition, or offline Mo's.
// ############################################################################
//
// PITFALLS:
//   1-INDEXED positions here; b[0] = 0 is the sentinel that makes b[l-1] valid.
//   Values must be in [lo, hi] as given to the constructor. Out-of-range values
//     recurse forever. Compress first if values can be negative or huge.
//   Memory is O(n log V) ints. n = 1e5 with V = 1e9 is 30 levels -> 3e6 ints,
//     fine; but building on uncompressed 1e18 values is not.
//   The recursion allocates with new and never frees. Fine for one test case;
//     for multi-test, rebuild into a fresh object and let it leak, or write a
//     destructor.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   k-th smallest in a[l..r]              kth()
//   median of a range                     kth(l, r, (r-l+2)/2)
//   how many <= x in a[l..r]              lte()
//   how many == x in a[l..r]              cnt()
//   how many in [x, y] in a[l..r]         lte(y) - lte(x-1)
//   rank of a[i] within a[l..r]           lte(l, r, a[i])
//   "how many smaller to the left"        lte over a prefix -- inversions
//   next value above x inside a range     binary search on lte()
//   range quantile / percentile           kth with k scaled
//
// NOT THIS: with updates -> merge sort tree / sqrt / offline. Sum of values in
//   a range -> prefix sums. Sum of values <= x in a range -> a wavelet tree
//   with an extra prefix-sum array per node (easy extension, not written here).
// ============================================================================
struct Wavelet {
    int lo, hi;
    Wavelet *L = nullptr, *R = nullptr;
    vector<int> b;                       // b[i] = # of first i elements going left

    // build over [from, to) with all values inside [x, y]. DESTROYS the range.
    Wavelet(int *from, int *to, int x, int y) : lo(x), hi(y) {
        if (from >= to) return;
        int mid = lo + (hi - lo) / 2;
        auto goes_left = [mid](int v) { return v <= mid; };
        b.reserve(to - from + 1);
        b.push_back(0);
        for (int *it = from; it != to; it++) b.push_back(b.back() + goes_left(*it));
        if (lo == hi) return;
        int *pivot = stable_partition(from, to, goes_left);   // STABLE, see box
        L = new Wavelet(from, pivot, lo, mid);
        R = new Wavelet(pivot, to, mid + 1, hi);
    }
    // k-th smallest in [l, r], 1-indexed positions and 1-indexed k
    int kth(int l, int r, int k) {
        if (l > r) return 0;
        if (lo == hi) return lo;
        int lb = b[l - 1], rb = b[r], inleft = rb - lb;
        if (k <= inleft) return L->kth(lb + 1, rb, k);
        return R->kth(l - lb, r - rb, k - inleft);
    }
    // how many elements of [l, r] are <= k
    int lte(int l, int r, int k) {
        if (l > r || k < lo) return 0;
        if (hi <= k) return r - l + 1;
        int lb = b[l - 1], rb = b[r];
        return L->lte(lb + 1, rb, k) + R->lte(l - lb, r - rb, k);
    }
    // how many elements of [l, r] equal k
    int cnt(int l, int r, int k) {
        if (l > r || k < lo || k > hi) return 0;
        if (lo == hi) return r - l + 1;
        int lb = b[l - 1], rb = b[r], mid = lo + (hi - lo) / 2;
        if (k <= mid) return L->cnt(lb + 1, rb, k);
        return R->cnt(l - lb, r - rb, k);
    }
};
