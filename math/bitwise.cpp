// ============================================================================
// BITWISE -- operations, identities, and the loops that live on them
// ----------------------------------------------------------------------------
// Three separate things in one file, because they get reached for together:
//   [A] the primitives and the builtins
//   [B] enumeration -- submasks, supermasks, k-subsets
//   [C] arithmetic expressed bitwise, and the identities worth memorising
//
// ############################################################################
// #  THE FOUR THAT ACTUALLY BITE
// #
// #   1.  1 << 63  IS UNDEFINED.  `1` is int. Write 1LL << 63, always. The
// #       same for any shift past 31 on an int -- including 1 << n inside a
// #       bitmask DP where n reached 32.
// #
// #   2.  __builtin_ctz(0) AND __builtin_clz(0) ARE UNDEFINED. They do not
// #       return 32, they return garbage that changes between compilers.
// #       Guard x != 0 before every single call.
// #
// #   3.  SHIFTING BY >= THE WIDTH IS UNDEFINED.  x << 64 on a 64-bit type is
// #       not 0. On x86 it is often x, because the CPU masks the shift count.
// #
// #   4.  & AND | BIND LOOSER THAN ==.  `a & 1 == 0` parses as `a & (1 == 0)`
// #       which is `a & 0` which is 0. Parenthesise: `(a & 1) == 0`.
// ############################################################################
//
// ----------------------------------------------------------------------------
// [A] PRIMITIVES
// ----------------------------------------------------------------------------
//   test bit i          (x >> i) & 1
//   set bit i           x |= 1 << i
//   clear bit i         x &= ~(1 << i)
//   toggle bit i        x ^= 1 << i
//   set i to v          x = (x & ~(1 << i)) | ((ll)v << i)
//
//   lowest set bit      x & -x            (the VALUE, e.g. 12 -> 4)
//   clear lowest set    x & (x - 1)       (12 -> 8)   <- the popcount loop
//   isolate lowest 0    ~x & (x + 1)
//   all bits below i    (1 << i) - 1
//   is a power of two   x && !(x & (x - 1))
//   round DOWN to pow2  1 << __lg(x)
//   round UP to pow2    x == 1 ? 1 : 1 << (__lg(x - 1) + 1)
//
//   BUILTINS -- add ll for 64-bit: __builtin_popcountll etc.
//     __builtin_popcount(x)   set bits
//     __builtin_ctz(x)        trailing zeros = index of lowest set bit
//     __builtin_clz(x)        leading zeros; 31 - clz(x) = index of highest
//     __builtin_parity(x)     popcount & 1
//     __lg(x)                 index of highest set bit, cleaner than 31-clz
//
// ----------------------------------------------------------------------------
// [C] IDENTITIES WORTH MEMORISING
// ----------------------------------------------------------------------------
//   x ^ x = 0        x ^ 0 = x        xor is its own inverse
//   a + b = (a ^ b) + 2*(a & b)        xor is add-without-carry, and is the
//   a + b = (a | b) + (a & b)          reason "sum vs xor" problems reduce to
//                                      "where do carries happen"
//   a | b = (a ^ b) + (a & b)
//   -x    = ~x + 1                     two's complement
//   x & -x                             lowest set bit, and why Fenwick works
//
//   XOR OF 1..n, closed form -- n%4: 0 -> n, 1 -> 1, 2 -> n+1, 3 -> 0
//     so xor over [l, r] is xor1n(r) ^ xor1n(l-1). Constant time.
//
//   nCr IS ODD  <=>  (n & r) == r        (Lucas mod 2 -- r is a submask of n)
//     so the number of odd entries in row n is 2^popcount(n).
//
//   a + b = (a ^ b) when a & b == 0      disjoint bits: sum == xor == or
//     This is the whole reason "partition into two sets with equal xor" and
//     "sum equals xor" problems are really about disjointness.
//
//   GRAY CODE   g(i) = i ^ (i >> 1)      consecutive codes differ in one bit
//     inverse: for (; j; j >>= 1) i ^= j;   (start j = g, i = g)
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS -- what bitwise turns into
// ----------------------------------------------------------------------------
//   subset DP over n <= 20 items        [B] iterate submasks, O(3^n)
//   "for each mask, sum over submasks"  SOS DP -- DP/optimization/sos_dp.cpp
//   maximum xor of a pair               binary trie, DS/Trie/binary_trie.cpp
//   max xor of a SUBSET                 xor basis, DP/sub-problems xorbasis.cpp
//   count pairs with a&b == 0           SOS over the complement
//   count pairs with a|b == full        same, complemented
//   count pairs by popcount             bucket by __builtin_popcount
//   Nim / impartial games               xor of Grundy numbers
//   "even number of every type"         parity vector, then xor basis
//   choose k of n, all combinations     [B] Gosper's hack
//   feasibility over a boolean DP       bitset, DS/bitset.cpp
//   subset sum reachable                bitset shifts, DS/bitset.cpp [1]
//   binary lifting / doubling           the exponent IS the bit decomposition
//   lowest set bit as a tree index      Fenwick, DS/BIT/
// ============================================================================

// ---------------------------------------------------------------------------
// [B] ENUMERATION
// ---------------------------------------------------------------------------

// every SUBMASK of mask, including mask and 0. O(2^popcount(mask)).
// Over all masks this is O(3^n), not O(4^n) -- each bit is in/out/absent.
void submasks(int mask) {
    for (int s = mask;; s = (s - 1) & mask) {
        // s is a submask of mask
        if (!s) break;
    }
}
// every SUPERMASK of mask within n bits. Also O(3^n) overall.
void supermasks(int mask, int n) {
    for (int s = mask; s < (1 << n); s = (s + 1) | mask) {
        // s contains mask
    }
}
// GOSPER'S HACK -- next integer with the same popcount.
// Walks every k-subset of n bits in increasing order.
//   for (int x = (1<<k)-1; x < (1<<n); x = next_same_popcount(x))
int next_same_popcount(int x) {
    int c = x & -x, r = x + c;
    return (((r ^ x) >> 2) / c) | r;
}

// xor of 0..n inclusive, O(1)
ll xor1n(ll n) {
    if (n < 0) return 0;
    switch (n & 3) {
        case 0: return n;
        case 1: return 1;
        case 2: return n + 1;
        default: return 0;
    }
}
ll xor_range(ll l, ll r) { return xor1n(r) ^ xor1n(l - 1); }

// nCr is odd  <=>  r is a submask of n
bool ncr_is_odd(ll n, ll r) { return (n & r) == r; }

// ---------------------------------------------------------------------------
// [C] ARITHMETIC WITHOUT + - * /
// ---------------------------------------------------------------------------
// Not faster than the operators -- the compiler emits better code. They are
// here because problems ASK for them ("no arithmetic operators"), and because
// the add loop is the clearest statement of what a carry is.
// ---------------------------------------------------------------------------
ll bit_add(ll a, ll b) {                 // xor is the sum, and is the carry
    while (b) {
        ll carry = (ll)((unsigned long long)(a & b) << 1);   // shift unsigned: signed overflow is UB
        a ^= b, b = carry;
    }
    return a;
}
ll bit_neg(ll a) { return bit_add(~a, 1); }
ll bit_sub(ll a, ll b) { return bit_add(a, bit_neg(b)); }

ll bit_mul(ll a, ll b) {                 // shift-and-add, the schoolbook method
    bool neg = (a < 0) ^ (b < 0);
    unsigned long long x = a < 0 ? -(unsigned long long)a : (unsigned long long)a;
    unsigned long long y = b < 0 ? -(unsigned long long)b : (unsigned long long)b;
    unsigned long long r = 0;
    for (; y; y >>= 1, x <<= 1)
        if (y & 1) r += x;
    return neg ? -(ll)r : (ll)r;
}
// floor division for NON-NEGATIVE inputs, by long division on bits
ll bit_div(ll a, ll b) {
    ll q = 0, rem = 0;
    for (int i = 62; i >= 0; i--) {
        rem = (rem << 1) | ((a >> i) & 1);
        if (rem >= b) rem -= b, q |= 1LL << i;
    }
    return q;
}
// branchless min/max: -(x < y) is all-ones when true, all-zeros when false
ll bit_min(ll x, ll y) { return y ^ ((x ^ y) & -(ll)(x < y)); }
ll bit_max(ll x, ll y) { return x ^ ((x ^ y) & -(ll)(x < y)); }
ll bit_abs(ll x) { ll m = x >> 63; return (x ^ m) - m; }

// gray code and back
ll gray(ll i) { return i ^ (i >> 1); }
ll ungray(ll g) { ll i = 0; for (ll j = g; j; j >>= 1) i ^= j; return i; }
