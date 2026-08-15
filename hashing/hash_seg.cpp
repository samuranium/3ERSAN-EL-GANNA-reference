// ============================================================================
// hash_seg.h  —  hashing UNDER UPDATES.  mod 2^61-1, one lane.
// ============================================================================
//
// WHAT THIS FILE DOES
//   Everything hash.h does for a static array, but with the array changing.
//   A prefix-hash array dies the moment a symbol changes; the fix is that a
//   hash is a MONOID, so it can live in a BIT or a segment tree.
//   Headline capability: PALINDROME QUERIES UNDER UPDATES, which Manacher
//   cannot do at all.
//
//   HashBIT  — point updates only. Two Fenwick trees. ~7x faster than the
//              segment tree and half the code. This is the default.
//   HashSeg  — use ONLY when you actually need range-assign.
//
// ----------------------------------------------------------------------------
// TYPES — what must be what, and why
// ----------------------------------------------------------------------------
//   `#define int long long` + `int32_t main()`.
//
//   long long is ENOUGH for hash values: they live in [0, 2^61-2], so
//     add()'s intermediate a+b < 2^62 < 2^63-1.
//   long long is REQUIRED for HashSeg::lazy, which stores an assigned VALUE
//     (up to 1e9 or beyond), not a hash. -1 is the "no tag" sentinel, so
//     assigned values must be >= 0.
//   __int128 is REQUIRED in mul(): 61-bit x 61-bit = 122-bit product.
//   unsigned long long is NOT needed anywhere in this file. There is no
//     SplitMix here, so `ull` never appears.
//
//   MEMORY: three tables now (pw, ipw, geo). At MAXN = 1e6+5 that is ~24 MB
//   as 8-byte entries. Shrink MAXN before you shrink the types.
//
// ----------------------------------------------------------------------------
// MINIMAL BUILDS — when you need X, copy Y
// ----------------------------------------------------------------------------
//   CORE (always, ~30 lines):
//     MOD, BASE, pw[], ipw[], add, sub, mul, power, enc, init_hash
//     (geo[] is only needed by HashSeg — drop it from init_hash for HashBIT)
//
//   point update + substring equality ... CORE + HashBIT{build, add_at,
//                                         set_at, pre_fwd, get}        ~30
//   + palindrome under updates .......... the above + bwd tree, pre_bwd,
//                                         get_rev, is_pal              +12
//   + suffix compare under updates ...... the above + lcp, compare     +15
//   range-assign anything ............... CORE + geo[] + HashSeg whole  ~90
//   read a symbol back out of HashSeg ... value_at (needs uniform())
//
//   If you only ever do point updates, do NOT copy HashSeg. It is 3x the
//   code and 7x slower for the same answers.
//
// ----------------------------------------------------------------------------
// THE ALGEBRA
// ----------------------------------------------------------------------------
//   (h1, L1) * (h2, L2) = (h1 + h2 * BASE^L1,  L1 + L2)
//   NOT commutative — order of children matters, never swap them.
//
//   The reverse hash rides in the same node with the operands swapped:
//     rev(A + B) = rev(B) + rev(A)  =>  rev = rev2 + rev1 * BASE^L2
//   Carrying both is what gives palindrome queries.
//
//   Range-assign is closed under this: "c repeated L times" has hash
//     enc(c) * geo[L],  geo[L] = 1 + B + B^2 + ... + B^(L-1)
//   so a lazy tag is just the assigned value.
//
// ----------------------------------------------------------------------------
// QUICK INDEX (both structures, 1-indexed inclusive, l > r means empty)
// ----------------------------------------------------------------------------
//   .set_at(p, c)          write value c at position p        BIT O(log n)
//   .get(l, r)             hash of a[l..r]                    BIT O(log n)
//   .get_rev(l, r)         hash of a[l..r] reversed           BIT O(log n)
//   .is_pal(l, r)          palindrome check under updates     BIT O(log n)
//   .lcp(i, j)             longest common prefix of suffixes  BIT O(log^2 n)
//   .compare(l1,r1,l2,r2)  lexicographic -1 / 0 / +1          BIT O(log^2 n)
//
//   HashSeg additionally:
//   .assign_range(l, r, c) set every position in [l, r] to c  O(log n)
//   .value_at(p)           read back the raw symbol           O(log n)
//   .node_of(l, r)         the merged node, if you need .len  O(log n)
//
// TRAPS
//   - merge() is not commutative. merge(left, right), never the other way.
//   - HashSeg's lazy tag uses -1 as "no tag": assigned values must be >= 0.
//   - HashSeg::lcp is O(log^2 n): a bisect on length times a tree query.
// ============================================================================

#include <bits/stdc++.h>
using namespace std;

#define int long long
#define el '\n'
#define fio ios_base::sync_with_stdio(0), cin.tie(0)

const int MAXN = 1e6 + 5;
const int MOD  = (1LL << 61) - 1;

int BASE;
int pw[MAXN];   // pw[i]  = BASE^i
int ipw[MAXN];  // ipw[i] = BASE^-i
int geo[MAXN];  // geo[i] = 1 + B + ... + B^(i-1); HashSeg only, drop for HashBIT

// a + b in the field. Safe in signed: a, b < 2^61 so a + b < 2^62.
int add(int a, int b) {
    a += b;
    return a >= MOD ? a - MOD : a;
}

int sub(int a, int b) {
    return add(a, MOD - b);
}

// a * b mod 2^61-1: 2^61 == 1 here, so high bits fold onto low bits.
// __int128 is REQUIRED — the product is up to 122 bits.
int mul(int a, int b) {
    __int128 c = (__int128)a * b;
    int lo = (int)(c & MOD);
    int hi = (int)(c >> 61);
    lo += hi;
    return lo >= MOD ? lo - MOD : lo;
}

int power(int b, int e) {
    int r = 1;
    for (; e; b = mul(b, b), e >>= 1)
        if (e & 1) r = mul(r, b);
    return r;
}

// Encode a raw value into [1, MOD-1]. Never 0, else "" == "\0" == "\0\0".
int enc(int x) {
    x %= MOD - 1;
    if (x < 0) x += MOD - 1;
    return x + 1;
}

// Call once in main. n = max sequence length.
void init_hash(int n) {
    mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count()
                   ^ (unsigned long long)(uintptr_t)&n);
    BASE = (int)(rng() % (unsigned long long)(MOD - 4)) + 2;
    int inv_base = power(BASE, MOD - 2);
    pw[0] = ipw[0] = 1;
    geo[0] = 0;
    for (int i = 1; i <= n; i++) {
        pw[i]  = mul(pw[i - 1], BASE);
        ipw[i] = mul(ipw[i - 1], inv_base);
        geo[i] = add(geo[i - 1], pw[i - 1]);
    }
}

// ----------------------------------------------------------------------------
// HashBIT — point update only. Two Fenwick trees over the same array.
//   fwd holds enc(a[i]) * BASE^(i-1)   -> forward hash
//   bwd holds enc(a[i]) * BASE^-(i-1)  -> reverse hash
// A reversed range is the backward prefix sum renormalised by BASE^(r-1)
// instead of BASE^-(l-1); that single sign flip is the whole trick.
// ----------------------------------------------------------------------------
struct HashBIT {
    int n;
    vector<int> a;    // raw values, kept so compare() can look at symbols
    vector<int> fwd;
    vector<int> bwd;

    HashBIT(const string &s) {
        int m = s.size();
        vector<int> v(m);
        for (int i = 0; i < m; i++) v[i] = (unsigned char)s[i];
        build(v);
    }

    HashBIT(const vector<int> &v) { build(v); }

    void build(const vector<int> &v) {
        n = v.size();
        a.assign(n + 1, 0);
        fwd.assign(n + 1, 0);
        bwd.assign(n + 1, 0);
        for (int i = 1; i <= n; i++) {
            a[i] = v[i - 1];
            add_at(i, enc(a[i]));
        }
    }

    // Add an already-encoded delta at position p, in both trees.
    void add_at(int p, int delta) {
        for (int i = p; i <= n; i += i & -i) {
            fwd[i] = add(fwd[i], mul(delta, pw[p - 1]));
            bwd[i] = add(bwd[i], mul(delta, ipw[p - 1]));
        }
    }

    // Overwrite position p with c: push the DIFFERENCE, not the value.
    void set_at(int p, int c) {
        int delta = sub(enc(c), enc(a[p]));
        a[p] = c;
        add_at(p, delta);
    }

    int pre_fwd(int i) const {
        int s = 0;
        for (; i > 0; i -= i & -i) s = add(s, fwd[i]);
        return s;
    }

    int pre_bwd(int i) const {
        int s = 0;
        for (; i > 0; i -= i & -i) s = add(s, bwd[i]);
        return s;
    }

    int get(int l, int r) const {
        if (l > r) return 0;
        return mul(sub(pre_fwd(r), pre_fwd(l - 1)), ipw[l - 1]);
    }

    int get_rev(int l, int r) const {
        if (l > r) return 0;
        return mul(sub(pre_bwd(r), pre_bwd(l - 1)), pw[r - 1]);
    }

    bool is_pal(int l, int r) const {
        return l > r || get(l, r) == get_rev(l, r);
    }

    // Bisect on match length; each probe is two O(log n) range hashes.
    int lcp(int i, int j) const {
        if (i == j) return n - i + 1;
        int lo = 0, hi = n + 1 - max(i, j);
        while (lo < hi) {
            int m = (lo + hi + 1) >> 1;
            if (get(i, i + m - 1) == get(j, j + m - 1)) lo = m;
            else hi = m - 1;
        }
        return lo;
    }

    int compare(int l1, int r1, int l2, int r2) const {
        int len1 = r1 - l1 + 1, len2 = r2 - l2 + 1;
        int cap = min(len1, len2);
        int k = min(lcp(l1, l2), cap);
        if (k == cap) return len1 == len2 ? 0 : (len1 < len2 ? -1 : 1);
        return a[l1 + k] < a[l2 + k] ? -1 : 1;
    }
};

// ----------------------------------------------------------------------------
// HashSeg — range-assign plus range query. The node carries forward hash,
// reverse hash, and length; the length is what makes merge well defined.
// ----------------------------------------------------------------------------
struct HashSeg {
    struct Node {
        int fwd;
        int rev;
        int len;
    };

    int n;
    vector<Node> t;
    vector<int> lazy;       // -1 means "no pending assignment"
    vector<int> init_vals;

    // Identity of the monoid: the empty sequence.
    static Node neutral() { return {0, 0, 0}; }

    // Node for "value c repeated L times". Forward and reverse are equal
    // because a constant run is its own reverse.
    static Node uniform(int c, int L) {
        int h = mul(enc(c), geo[L]);
        return {h, h, L};
    }

    // Concatenate left then right. NOT commutative.
    //   fwd: right block sits above BASE^len(left)
    //   rev: reversed order, so left block sits above BASE^len(right)
    static Node merge(const Node &A, const Node &B) {
        if (!A.len) return B;
        if (!B.len) return A;
        return { add(A.fwd, mul(B.fwd, pw[A.len])),
                 add(B.rev, mul(A.rev, pw[B.len])),
                 A.len + B.len };
    }

    HashSeg(const string &s) {
        int m = s.size();
        vector<int> v(m);
        for (int i = 0; i < m; i++) v[i] = (unsigned char)s[i];
        build(v);
    }

    HashSeg(const vector<int> &v) { build(v); }

    void build(const vector<int> &v) {
        n = v.size();
        t.assign(4 * n + 4, neutral());
        lazy.assign(4 * n + 4, -1);
        init_vals.assign(n + 1, 0);
        for (int i = 1; i <= n; i++) init_vals[i] = v[i - 1];
        if (n) build_rec(1, 1, n);
        init_vals.clear();
        init_vals.shrink_to_fit();
    }

    void build_rec(int x, int l, int r) {
        if (l == r) { t[x] = uniform(init_vals[l], 1); return; }
        int m = (l + r) >> 1;
        build_rec(2 * x, l, m);
        build_rec(2 * x + 1, m + 1, r);
        t[x] = merge(t[2 * x], t[2 * x + 1]);
    }

    void apply(int x, int l, int r, int c) {
        t[x] = uniform(c, r - l + 1);
        lazy[x] = c;
    }

    void push(int x, int l, int r) {
        if (lazy[x] < 0) return;
        int m = (l + r) >> 1;
        apply(2 * x, l, m, lazy[x]);
        apply(2 * x + 1, m + 1, r, lazy[x]);
        lazy[x] = -1;
    }

    void update(int x, int l, int r, int ql, int qr, int c) {
        if (qr < l || r < ql) return;
        if (ql <= l && r <= qr) { apply(x, l, r, c); return; }
        push(x, l, r);
        int m = (l + r) >> 1;
        update(2 * x, l, m, ql, qr, c);
        update(2 * x + 1, m + 1, r, ql, qr, c);
        t[x] = merge(t[2 * x], t[2 * x + 1]);
    }

    Node query(int x, int l, int r, int ql, int qr) {
        if (qr < l || r < ql) return neutral();
        if (ql <= l && r <= qr) return t[x];
        push(x, l, r);
        int m = (l + r) >> 1;
        return merge(query(2 * x, l, m, ql, qr),
                     query(2 * x + 1, m + 1, r, ql, qr));
    }

    void set_at(int p, int c) { update(1, 1, n, p, p, c); }

    void assign_range(int l, int r, int c) {
        if (l <= r) update(1, 1, n, l, r, c);
    }

    Node node_of(int l, int r) {
        return l > r ? neutral() : query(1, 1, n, l, r);
    }

    int get(int l, int r)     { return node_of(l, r).fwd; }
    int get_rev(int l, int r) { return node_of(l, r).rev; }

    // One query, then compare the two halves of the same node. No second walk.
    bool is_pal(int l, int r) {
        if (l > r) return true;
        Node z = node_of(l, r);
        return z.fwd == z.rev;
    }

    // Read back a raw symbol. A length-1 node's hash is enc(c) = c + 1
    // for c >= 0, so undo the +1.
    int value_at(int p) { return node_of(p, p).fwd - 1; }

    // O(log^2 n): bisect on length, each probe is a tree query.
    int lcp(int i, int j) {
        if (i == j) return n - i + 1;
        int lo = 0, hi = n + 1 - max(i, j);
        while (lo < hi) {
            int m = (lo + hi + 1) >> 1;
            if (get(i, i + m - 1) == get(j, j + m - 1)) lo = m;
            else hi = m - 1;
        }
        return lo;
    }

    int compare(int l1, int r1, int l2, int r2) {
        int len1 = r1 - l1 + 1, len2 = r2 - l2 + 1;
        int cap = min(len1, len2);
        int k = min(lcp(l1, l2), cap);
        if (k == cap) return len1 == len2 ? 0 : (len1 < len2 ? -1 : 1);
        return value_at(l1 + k) < value_at(l2 + k) ? -1 : 1;
    }
};

int32_t main() {
    fio;
    init_hash(MAXN - 1);   // shrink MAXN when n is small

    return 0;
}
