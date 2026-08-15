// ============================================================================
// MODULAR ARITHMETIC -- the canonical add / subt / mult / fp / divi
// ----------------------------------------------------------------------------
// This set is copy-pasted into nCr_with_mod.cpp, matrix expo.cpp, incexc.cpp
// and hashing/hash-vec.cpp. This is the one to copy FROM.
//
// ############################################################################
// #  divi() NEEDS A PRIME MODULUS
// #
// #  fp(b, mod-2) is Fermat's little theorem and it is only an inverse when
// #  mod is PRIME and b is not a multiple of it.
// #      mod = 1e9+7      prime, fine
// #      mod = 998244353  prime, fine
// #      mod = 1e9        NOT prime -- divi silently returns nonsense
// #  Non-prime modulus, or you need 1/b where gcd(b,mod) may not be 1:
// #  use math/extgcd.cpp inv_mod(), which reports failure instead of guessing.
// ############################################################################
//
// WHY THE BRANCHES: (a += b) < mod ? a : a - mod is one compare instead of a
//   division. Only valid because both inputs are already reduced -- feed it a
//   value >= mod and it stays >= mod. Reduce at the boundary, not everywhere.
//
// PITFALLS:
//   fp()'s exponent is ll. mod-2 fits in int, but a raw exponent from the
//     problem (n up to 1e18) does not. Never narrow it.
//   mult() is 1ll*a*b -- the cast must be on the FIRST operand, before the
//     multiply, or the product overflows before it is widened.
//   mod > 2^31 breaks mult(): 1ll*a*b overflows. Use mulmod() below.
//   Negative inputs: subt() assumes both are already in [0, mod). Normalise
//     with ((x % mod) + mod) % mod at the point where the value enters.
//   inv_table only works for i < mod and prime mod.
//
// SUBPROBLEMS:
//   a^e mod p                      fp(a, e)
//   a^-1 mod p, p prime            fp(a, mod-2)   or divi(1, a)
//   a^-1 mod m, any m              math/extgcd.cpp inv_mod()
//   all inverses 1..n in O(n)      inv_table()
//   nCr mod p                      combinatorics/nCr_with_mod.cpp
//   a^e where e is huge            e mod (p-1) by Fermat, ONLY if gcd(a,p)=1
//   mod is not prime and huge      mulmod + extgcd, or CRT over its factors
//                                  (math/crt.cpp)
// ============================================================================
const int mod = 1'000'000'007;   // 998'244'353 == 119<<23|1 for NTT problems

int add(int a, int b) { return (a += b) < mod ? a : a - mod; }
int subt(int a, int b) { return (a -= b) < 0 ? a + mod : a; }
int mult(int a, int b) { return 1ll * a * b % mod; }

int fp(int b, ll e) {            // exponent is ll on purpose
    int r = 1;
    for (; e; b = mult(b, b), e >>= 1)
        if (e & 1) r = mult(r, b);
    return r;
}
int divi(int a, int b) { return mult(a, fp(b, mod - 2)); }   // PRIME mod only

// every inverse 1..n in O(n). inv[i] = -(mod/i) * inv[mod%i]
void inv_table(int n, vector<int> &inv) {
    inv.assign(n + 1, 1);
    for (int i = 2; i <= n; i++)
        inv[i] = mult(mod - mod / i, inv[mod % i]);
}
// use when mod does not fit in 32 bits -- 1ll*a*b would overflow
ll mulmod(ll a, ll b, ll m) { return (__int128)a * b % m; }
ll fp_big(ll b, ll e, ll m) {
    ll r = 1 % m;
    for (b %= m; e; b = mulmod(b, b, m), e >>= 1)
        if (e & 1) r = mulmod(r, b, m);
    return r;
}
