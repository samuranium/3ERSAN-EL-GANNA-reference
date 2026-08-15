// ============================================================================
// FWHT -- xor / and / or convolution in O(n log n), n a power of two
// ----------------------------------------------------------------------------
//   c[k] = sum over i OP j == k of a[i] * b[j],   OP in { xor, and, or }
//
// Same shape as FFT -- transform both, multiply pointwise, transform back --
// but the transform is different and far simpler: no roots of unity, no
// complex numbers, no precision. Just add/subtract in place.
//
// ############################################################################
// #  THIS IS NOT FFT, AND FFT IS NOT THIS
// #
// #  FFT/NTT convolves by the SUM of indices  (i + j).
// #  FWHT convolves by a BITWISE op            (i ^ j, i & j, i | j).
// #
// #  Reaching for FFT on an xor problem is the standard mistake. The index
// #  set here is a hypercube, not a line, and the size must be a power of two
// #  covering the whole value range -- never n + m - 1.
// ############################################################################
//
// ############################################################################
// #  ONLY THE XOR INVERSE DIVIDES BY n
// #
// #    xor  forward (u+v, u-v)      inverse: same, then divide everything by n
// #    or   forward a[hi] += a[lo]  inverse: a[hi] -= a[lo]
// #    and  forward a[lo] += a[hi]  inverse: a[lo] -= a[hi]
// #
// #  and/or are their own inverses up to the sign, with NO division. Dividing
// #  them by n is the usual copy-paste error and scales the answer wrong.
// ############################################################################
//
// PITFALLS:
//   Size must be a POWER OF TWO >= 2^(bits in the largest value). Pad with 0.
//   Working mod p: replace + and - with add/subt, and the xor inverse's
//     division by n with multiplication by n^-1. Never integer-divide mod p.
//   Values grow: xor convolution of two arrays with entries up to 1e9 and
//     n = 2^20 overflows ll during the transform. Reduce mod p as you go, or
//     bound the magnitudes first.
//   Self-convolution (a with itself) is one transform, square pointwise, one
//     inverse -- do not transform twice.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   # of pairs with a[i] ^ a[j] == k     xor convolve the count array with itself
//   # of pairs with a[i] & a[j] == k     and convolution
//   # of pairs with a[i] | a[j] == k     or convolution
//   # of pairs with a[i] & a[j] == 0     the k = 0 entry of the and transform
//   ... or via SOS over complements      DP/sos.cpp -- often simpler
//   k-fold xor convolution               transform once, take the k-th power
//                                        pointwise, transform back
//   "choose a subset, xor is x"          xor convolution of (1 + z^a_i) terms,
//                                        or an xor basis if you only need
//                                        reachability -- DP/sub-problems xorbasis.cpp
//   xor-sum over all subsets             usually the basis, not this
//   SUBSET-SUM convolution (disjoint     ranked transform: add a "popcount
//     union only, i & j == 0)            layer" dimension, or convolution
//                                        over 2^n * n
//
// NOT THIS: sums of indices -> math/fft_ntt.cpp. Aggregating over submasks
//   (not convolving) -> SOS DP, DP/sos.cpp, which is O(2^n * n) and simpler.
// ============================================================================

// dir = +1 forward, -1 inverse
void fwht_xor(vector<ll> &a, int dir) {
    int n = a.size();
    for (int len = 1; len < n; len <<= 1)
        for (int i = 0; i < n; i += len << 1)
            for (int j = 0; j < len; j++) {
                ll u = a[i + j], v = a[i + j + len];
                a[i + j] = u + v, a[i + j + len] = u - v;
            }
    if (dir < 0)
        for (ll &x : a) x /= n;                 // ONLY xor divides -- see the box
}
void fwht_or(vector<ll> &a, int dir) {
    int n = a.size();
    for (int len = 1; len < n; len <<= 1)
        for (int i = 0; i < n; i += len << 1)
            for (int j = 0; j < len; j++)
                a[i + j + len] += dir * a[i + j];
}
void fwht_and(vector<ll> &a, int dir) {
    int n = a.size();
    for (int len = 1; len < n; len <<= 1)
        for (int i = 0; i < n; i += len << 1)
            for (int j = 0; j < len; j++)
                a[i + j] += dir * a[i + j + len];
}
// op: 0 = xor, 1 = or, 2 = and.  a and b must already be the same power of two.
vector<ll> convolve_bitwise(vector<ll> a, vector<ll> b, int op) {
    auto T = [&](vector<ll> &v, int d) {
        if (op == 0) fwht_xor(v, d);
        else if (op == 1) fwht_or(v, d);
        else fwht_and(v, d);
    };
    T(a, 1), T(b, 1);
    for (size_t i = 0; i < a.size(); i++) a[i] *= b[i];
    T(a, -1);
    return a;
}
