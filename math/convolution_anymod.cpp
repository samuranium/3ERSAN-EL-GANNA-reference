// ============================================================================
// CONVOLUTION MODULO ANYTHING -- three NTTs plus CRT, exact
// ----------------------------------------------------------------------------
// There is no NTT modulo 1e9+7. The fix: convolve under THREE different
// NTT-friendly primes, then reconstruct each coefficient by CRT. The true
// value is bounded by n * (mod-1)^2 < 1e6 * 1e18 = 1e24, and the three primes
// multiply to ~2.2e26, so the reconstruction is exact.
//
// ############################################################################
// #  WHY THREE AND NOT TWO
// #
// #  Two primes cover ~9.4e17 -- less than a single coefficient can reach
// #  (n * (p-1)^2 is around 1e24 for n = 1e6, p = 1e9). Two is enough only if
// #  you can bound the coefficients yourself; three is enough always.
// #
// #  Fewer primes is the standard silent failure: correct on small tests,
// #  wrong once a coefficient exceeds the product.
// ############################################################################
//
// THE PRIMES, all of the form c*2^k + 1 with primitive root 3:
//   998244353  = 119 * 2^23 + 1     supports length up to 2^23
//   1004535809 = 479 * 2^21 + 1     up to 2^21
//   469762049  =   7 * 2^26 + 1     up to 2^26
// The transform length is capped by the SMALLEST of these, so n <= 2^21.
//
// PITFALLS:
//   Reduce inputs mod your target M first; negative or oversized inputs break
//     the bound argument.
//   The Garner step below uses __int128 -- it must, the intermediate exceeds
//     ll.
//   3x the work of a single NTT. If your modulus happens to be 998244353, do
//     not use this file; use math/fft_ntt.cpp directly.
//   Alternative: split each coefficient into high/low 15 bits and run 4
//     complex FFTs. Faster, fiddlier, and still has a precision budget. Prefer
//     this version unless the time limit is genuinely tight.
//
// SUBPROBLEMS: the same list as math/fft_ntt.cpp -- polynomial multiplication,
//   counting pairs by sum, big integers, string matching with wildcards -- but
//   whenever the answer must come out modulo an arbitrary M.
// ============================================================================
namespace anymod {
const int P[3] = {998244353, 1004535809, 469762049};

int pw(int b, ll e, int m) {
    int r = 1;
    for (b %= m; e; b = (ll)b * b % m, e >>= 1)
        if (e & 1) r = (ll)r * b % m;
    return r;
}
void ntt(vector<int> &a, bool inv, int m) {
    int n = a.size();
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }
    for (int len = 2; len <= n; len <<= 1) {
        int w = pw(3, (m - 1) / len, m);
        if (inv) w = pw(w, m - 2, m);
        for (int i = 0; i < n; i += len) {
            int wn = 1;
            for (int j = 0; j < len / 2; j++) {
                int u = a[i + j], v = (ll)a[i + j + len / 2] * wn % m;
                a[i + j] = u + v < m ? u + v : u + v - m;
                a[i + j + len / 2] = u - v >= 0 ? u - v : u - v + m;
                wn = (ll)wn * w % m;
            }
        }
    }
    if (inv) {
        int ninv = pw(n, m - 2, m);
        for (int &x : a) x = (ll)x * ninv % m;
    }
}
vector<int> mul_mod(vector<int> a, vector<int> b, int m) {
    int need = a.size() + b.size() - 1, n = 1;
    while (n < need) n <<= 1;
    a.resize(n), b.resize(n);
    for (int &x : a) x %= m;
    for (int &x : b) x %= m;
    ntt(a, false, m), ntt(b, false, m);
    for (int i = 0; i < n; i++) a[i] = (ll)a[i] * b[i] % m;
    ntt(a, true, m);
    a.resize(need);
    return a;
}
// Garner: recover x mod M from x mod P[0], P[1], P[2]
int garner(int r0, int r1, int r2, int M) {
    // x = r0 + P0*t1 + P0*P1*t2
    ll inv01 = pw(P[0] % P[1], P[1] - 2, P[1]);
    ll t1 = (ll)((r1 - r0) % P[1] + P[1]) % P[1] * inv01 % P[1];
    ll inv02 = pw((ll)P[0] % P[2] * (P[1] % P[2]) % P[2], P[2] - 2, P[2]);
    ll cur = ((__int128)P[0] * t1 + r0) % P[2];
    ll t2 = (ll)((r2 - cur) % P[2] + P[2]) % P[2] * inv02 % P[2];
    __int128 x = (__int128)r0 + (__int128)P[0] * t1
               + (__int128)P[0] * P[1] % M * t2;
    return (int)(x % M);
}
// convolution of a and b, every coefficient modulo M. M may be ANY modulus.
vector<int> convolve(const vector<int> &a, const vector<int> &b, int M) {
    if (a.empty() || b.empty()) return {};
    vector<int> c0 = mul_mod(a, b, P[0]);
    vector<int> c1 = mul_mod(a, b, P[1]);
    vector<int> c2 = mul_mod(a, b, P[2]);
    vector<int> res(c0.size());
    for (size_t i = 0; i < c0.size(); i++) res[i] = garner(c0[i], c1[i], c2[i], M);
    return res;
}
}  // namespace anymod
