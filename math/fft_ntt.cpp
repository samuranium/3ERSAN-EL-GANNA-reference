// ============================================================================
// FFT / NTT -- convolution in O(n log n)
// ----------------------------------------------------------------------------
// c[k] = sum over i+j==k of a[i]*b[j].  Any problem that is "combine every
// pair and index by the sum" is a convolution, whether or not it mentions
// polynomials.
//
// ############################################################################
// #  PICK THE RIGHT ONE. THIS IS THE DECISION, NOT THE CODE.
// #
// #   mod is 998244353 (or any p = c*2^k + 1)   -> NTT.  Exact. Use this.
// #   answers fit in ll and no mod at all       -> NTT anyway, mod is big
// #                                                enough; or FFT if values
// #                                                are small
// #   mod is 1e9+7 or arbitrary                 -> see ARBITRARY MOD below.
// #                                                Plain FFT will NOT survive
// #                                                the precision.
// #   real / floating data (geometry, signal)   -> FFT
// #
// #  998244353 = 119 * 2^23 + 1, so it supports transforms up to length 2^23.
// #  1e9+7 is NOT of that form -- there is no NTT modulo 1e9+7, which is the
// #  single most common misunderstanding here.
// ############################################################################
//
// ############################################################################
// #  FFT PRECISION IS A HARD LIMIT
// #
// #  doubles hold ~53 bits. The largest intermediate is about
// #        n * max|a| * max|b|
// #  and once that passes ~9e15 the rounding is bigger than 0.5 and llround()
// #  returns the wrong integer. At n = 1e6 that means values up to ~3e4 are
// #  safe and 1e9 is nowhere close.
// #  Symptom: answers correct on samples, off by one or two on big tests.
// ############################################################################
//
// PITFALLS:
//   Resize to a POWER OF TWO that is >= |a| + |b| - 1. Too small and the
//     result wraps around cyclically -- which is silent, and is exactly what
//     you want if you actually need a cyclic convolution.
//   The inverse transform is the forward one with conjugated roots plus a
//     division by n. Forgetting the division gives everything scaled by n.
//   Bit-reversal must happen before the butterflies, not after.
//   NTT input must already be reduced mod p; negative values wrap wrongly.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS -- everything that is secretly a convolution
// ----------------------------------------------------------------------------
//   polynomial multiplication            multiply()
//   big integer multiplication           digits are coefficients, then carry
//   count pairs (i,j) with a[i]+b[j]=k   convolve the count arrays
//   count pairs with a[i]-b[j]=k         reverse one side first
//   # of ways to reach each sum with     convolve the item-count arrays; with
//     k items from a multiset            k items, take the k-th power
//   string matching with wildcards       sum over shifts of
//                                        (s[i]-p[j])^2 * s[i] * p[j]; expand
//                                        into three convolutions
//   all pairwise distances / sums        convolve the indicator arrays
//   subset sum COUNT (not feasibility)   convolution; feasibility is a bitset,
//                                        DS/bitset.cpp
//   xor / and / or convolution           NOT this -- that is FWHT, a different
//                                        transform with the same shape
//   polynomial inverse / log / exp /     Newton iteration on top of this;
//     division / sqrt                    each is O(n log n)
//   linear recurrence, huge n            Kitamasa / Bostan-Mori on top of this
//
// ARBITRARY MOD (e.g. 1e9+7), two standard routes:
//   1. THREE NTTs modulo three different NTT primes, then CRT the results.
//      Exact, ~3x the work, no precision worries. Preferred.
//   2. SPLIT each coefficient into high and low 15 bits and run 4 (or 3, with
//      the conjugate trick) complex FFTs. Faster, fiddlier, and still has a
//      precision budget.
// ============================================================================

// ---------------------------------------------------------------------------
// NTT -- exact, modulo a prime of the form c*2^k + 1
// ---------------------------------------------------------------------------
const int NMOD = 998244353, NROOT = 3;      // 119 * 2^23 + 1

int npow(int b, ll e) {
    int r = 1;
    for (; e; b = (ll)b * b % NMOD, e >>= 1)
        if (e & 1) r = (ll)r * b % NMOD;
    return r;
}
void ntt(vector<int> &a, bool inv) {
    int n = a.size();
    for (int i = 1, j = 0; i < n; i++) {            // bit reversal
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }
    for (int len = 2; len <= n; len <<= 1) {
        int w = npow(NROOT, (NMOD - 1) / len);
        if (inv) w = npow(w, NMOD - 2);
        for (int i = 0; i < n; i += len) {
            int wn = 1;
            for (int j = 0; j < len / 2; j++) {
                int u = a[i + j], v = (ll)a[i + j + len / 2] * wn % NMOD;
                a[i + j] = u + v < NMOD ? u + v : u + v - NMOD;
                a[i + j + len / 2] = u - v >= 0 ? u - v : u - v + NMOD;
                wn = (ll)wn * w % NMOD;
            }
        }
    }
    if (inv) {
        int ninv = npow(n, NMOD - 2);
        for (int &x : a) x = (ll)x * ninv % NMOD;
    }
}
vector<int> multiply(vector<int> a, vector<int> b) {
    if (a.empty() || b.empty()) return {};
    int need = a.size() + b.size() - 1, n = 1;
    while (n < need) n <<= 1;
    a.resize(n), b.resize(n);
    ntt(a, false), ntt(b, false);
    for (int i = 0; i < n; i++) a[i] = (ll)a[i] * b[i] % NMOD;
    ntt(a, true);
    a.resize(need);
    return a;
}

// ---------------------------------------------------------------------------
// FFT -- doubles. Only for real data, or small integers. Read the box.
// ---------------------------------------------------------------------------
typedef complex<double> cd;

void fft(vector<cd> &a, bool inv) {
    int n = a.size();
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }
    for (int len = 2; len <= n; len <<= 1) {
        double ang = 2 * acos(-1.0) / len * (inv ? -1 : 1);
        cd w(cos(ang), sin(ang));
        for (int i = 0; i < n; i += len) {
            cd wn(1);
            for (int j = 0; j < len / 2; j++) {
                cd u = a[i + j], v = a[i + j + len / 2] * wn;
                a[i + j] = u + v;
                a[i + j + len / 2] = u - v;
                wn *= w;
            }
        }
    }
    if (inv)
        for (cd &x : a) x /= n;
}
vector<ll> multiply_fft(const vector<ll> &A, const vector<ll> &B) {
    if (A.empty() || B.empty()) return {};
    int need = A.size() + B.size() - 1, n = 1;
    while (n < need) n <<= 1;
    vector<cd> a(n), b(n);
    for (int i = 0; i < (int)A.size(); i++) a[i] = cd((double)A[i], 0);
    for (int i = 0; i < (int)B.size(); i++) b[i] = cd((double)B[i], 0);
    fft(a, false), fft(b, false);
    for (int i = 0; i < n; i++) a[i] *= b[i];
    fft(a, true);
    vector<ll> res(need);
    for (int i = 0; i < need; i++) res[i] = llround(a[i].real());
    return res;
}
