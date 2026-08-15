// ============================================================================
// hash.h  —  rolling hash over a STATIC sequence.  mod 2^61-1, one lane.
// ============================================================================
//
// WHAT THIS FILE DOES
//   Gives you the hash of ANY substring / subarray in O(1), after O(n) setup.
//   From that one primitive: substring equality, palindrome tests, pattern
//   search, longest common prefix of two suffixes, lexicographic compare,
//   borders, periods, distinct-substring counts, longest common substring.
//   Plus Manacher and Booth, which are strictly sharper than hashing for
//   palindromes and least rotation respectively.
//
//   Works on strings AND on integer arrays with values up to 1e9 (any
//   |x| < 2.3e18). No compression, no alphabet setup — enc() folds them.
//
// ----------------------------------------------------------------------------
// TYPES — what must be what, and why
// ----------------------------------------------------------------------------
//   `#define int long long` + `int32_t main()`. Every hash, index and count
//   below is 64-bit.
//
//   long long is ENOUGH for hash values, and this is not luck:
//     a hash lives in [0, MOD-1] = [0, 2^61-2], so add()'s intermediate
//     a+b < 2^62 < 2^63-1. Two bits of headroom, exactly what the
//     conditional subtract needs. Never widen this.
//
//   __int128 is REQUIRED in mul(): two 61-bit factors make a 122-bit product.
//     Signed __int128 is fine because both factors are non-negative, so the
//     `>> 61` is not an arithmetic-shift trap.
//
//   unsigned long long is REQUIRED in SplitMix and NOWHERE else in this file.
//     splitmix64 needs defined wraparound at 2^64 and a LOGICAL right shift.
//     On a signed type the multiplies are UB and `>>` sign-extends, so the
//     mixer stops mixing. If you drop SplitMix, you can delete `ull` entirely.
//
//   int32_t would be enough for every index here. Not worth the risk; the
//     only cost is the two power tables, and those are 8 bytes either way.
//
// ----------------------------------------------------------------------------
// MINIMAL BUILDS — when you need X, copy Y
// ----------------------------------------------------------------------------
//   CORE (always, ~30 lines):
//     MOD, BASE, pw[], ipw[], add, sub, mul, power, enc, init_hash
//
//   substring equality only ............ CORE + Hash{build, get}        ~15
//   pattern search (Rabin-Karp) ........ the above + hash_of, occurrences  +8
//   palindrome of a static range ....... CORE + Hash{build, get, get_rev,
//                                                    is_pal}            ~20
//   suffix comparison / sorting ........ CORE + Hash{build, get, lcp,
//                                                    compare}           ~25
//   borders / periods .................. CORE + Hash{build, get} +
//                                        borders, longest_border, period ~20
//   count distinct substrings .......... CORE + Hash{build,get} +
//                                        SplitMix, HashSet, distinct     ~25
//   longest common substring of A,B .... CORE + Hash{build,get} +
//                                        SplitMix, longest_common_substring
//   longest / count of palindromes ..... manacher + longest_pal / count_pal
//                                        ONLY. No CORE, no Hash at all.   ~25
//   least rotation ..................... min_rotation ONLY. No CORE.       ~10
//   "is t a rotation of s" ............. CORE + Hash + occurrences +
//                                        is_rotation, and MAXN >= 2n
//
//   Note the last three: Manacher and Booth are self-contained. If the
//   problem is only "longest palindromic substring", you do not need any of
//   the modular arithmetic in this file.
//
// ----------------------------------------------------------------------------
// THE MODEL (memorise this one line; everything else follows)
// ----------------------------------------------------------------------------
//   hash(l..r) = sum over i of enc(a[l+i]) * BASE^i
//   i.e. every substring is normalised so its FIRST symbol sits on BASE^0.
//   Consequence: hash(A + B) = hash(A) + hash(B) * BASE^|A|
//
//   All ranges are 1-indexed and inclusive. l > r is legal, means empty,
//   hashes to 0.
//
// ----------------------------------------------------------------------------
// QUICK INDEX
// ----------------------------------------------------------------------------
//   init_hash(MAXN - 1)         once in main, before anything else
//   Hash h(s);  Hash h(v);      v is vector<int> (= vector<long long>)
//
//   h.get(l, r)                 hash of a[l..r]                        O(1)
//   h.get_rev(l, r)             hash of a[l..r] reversed               O(1)
//   h.equal(l1,r1,l2,r2)        are the two ranges identical?          O(1)
//   h.is_pal(l, r)              is a[l..r] a palindrome?               O(1)
//   h.lcp(i, j)                 longest common prefix of suffixes i,j  O(log n)
//   h.lcs(i, j)                 longest common suffix of prefixes i,j  O(log n)
//   h.compare(l1,r1,l2,r2)      lexicographic -1 / 0 / +1              O(log n)
//   h.borders()                 all border lengths                     O(n log n)
//   h.longest_border()          longest proper border                  O(n)
//   h.period()                  smallest period                        O(n)
//   h.has_period(p)             is p a period?                         O(1)
//   h.longest_pal_prefix()      longest palindromic prefix             O(n)
//   h.longest_pal_suffix()      longest palindromic suffix             O(n)
//   h.occurrences(t)            all start positions of t               O(n)
//   h.distinct(len)             # distinct substrings of that length   O(n) exp
//   h.distinct_all()            # distinct substrings, all lengths     O(n^2) exp
//   h.freq(len)                 hash -> count map for that length      O(n log n)
//
//   lce(A, i, B, j)             longest common extension ACROSS two    O(log n)
//   longest_common_substring(A, B, &pa, &pb)                    O((n+m) log) exp
//   max_overlap(A, B)           longest suffix(A) that is prefix(B)    O(min)
//   min_rotation(s)             start of least rotation (Booth)        O(n)
//   is_rotation(s, t)           is t a rotation of s?                  O(n)
//   manacher(s, d1, d2)         palindromic radii, odd and even        O(n)
//   longest_pal(s)              {l, r} of a longest palindrome         O(n)
//   count_pal(s)                # palindromic substrings               O(n)
//
// ----------------------------------------------------------------------------
// SIZING
//   MAXN is the only knob. init_hash(MAXN - 1), NEVER init_hash(MAXN) — the
//   loop writes pw[n], so passing MAXN runs one past the end.
//   MAXN = 1e6+5 costs ~16 MB (two 8-byte tables). Shrink it when n is small.
//   Doubling a string for rotation tricks needs MAXN >= 2n.
//
// TRAPS
//   - The base is randomised per run. A fixed base is exactly what anti-hash
//     tests target. Do not hardcode it.
//   - enc() never returns 0, otherwise "" == "\0" == "\0\0".
//   - max_overlap is NOT monotone in k, so binary search on it is wrong.
//     Palindromic radius IS monotone, so bisecting there is fine.
//   - For longest / count of palindromes use Manacher, O(n), not hash plus
//     bisect, O(n log n). Both are here; take the sharper one.
// ============================================================================

#include <bits/stdc++.h>
using namespace std;

#define int long long
#define el '\n'
#define ull unsigned long long   // ONLY for SplitMix; see TYPES above
#define fio ios_base::sync_with_stdio(0), cin.tie(0)

const int MAXN = 1e6 + 5;          // >= longest sequence you will hash
const int MOD  = (1LL << 61) - 1;  // Mersenne prime 2^61 - 1

int BASE;                          // randomised in init_hash
int pw[MAXN];                      // pw[i]  = BASE^i
int ipw[MAXN];                     // ipw[i] = BASE^-i

// a + b in the field. One conditional subtract beats a modulo.
// Safe in signed: a, b < 2^61 so a + b < 2^62, well inside long long.
int add(int a, int b) {
    a += b;
    return a >= MOD ? a - MOD : a;
}

// a - b in the field, as a + (MOD - b) so we never go negative.
int sub(int a, int b) {
    return add(a, MOD - b);
}

// a * b mod 2^61-1. The reason this modulus is worth using:
// 2^61 == 1 here, so x mod MOD == (low 61 bits) + (high bits).
// One 128-bit multiply, one shift, one add, one conditional subtract.
// __int128 is REQUIRED: the product is up to 122 bits.
int mul(int a, int b) {
    __int128 c = (__int128)a * b;
    int lo = (int)(c & MOD);
    int hi = (int)(c >> 61);
    lo += hi;
    return lo >= MOD ? lo - MOD : lo;
}

// b^e mod MOD. e can be as large as MOD-2 ~ 2.3e18, which is why it is long long.
int power(int b, int e) {
    int r = 1;
    for (; e; b = mul(b, b), e >>= 1)
        if (e & 1) r = mul(r, b);
    return r;
}

// Encode one raw value into a symbol in [1, MOD-1].
// Fold into MOD-1 residues, then +1 so 0 is never produced: a symbol worth 0
// contributes nothing, which makes "" == "\0" == "\0\0".
int enc(int x) {
    x %= MOD - 1;
    if (x < 0) x += MOD - 1;
    return x + 1;
}

// Call once in main. n = max sequence length. Fills both power tables.
void init_hash(int n) {
    mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count()
                   ^ (ull)(uintptr_t)&n);
    BASE = (int)(rng() % (ull)(MOD - 4)) + 2;   // random base: fixed is hackable
    int inv_base = power(BASE, MOD - 2);        // Fermat: b^(p-2) == b^-1
    pw[0] = ipw[0] = 1;
    for (int i = 1; i <= n; i++) {
        pw[i]  = mul(pw[i - 1], BASE);
        ipw[i] = mul(ipw[i - 1], inv_base);
    }
}

// splitmix64 — a real mixer for unordered_set. std::hash on integers is the
// identity, which is what anti-hash tests on unordered_* exploit.
// This is the ONLY place unsigned long long is required: the algorithm needs
// wraparound at 2^64 and a logical right shift.
struct SplitMix {
    size_t operator()(int x) const {
        ull z = (ull)x + 0x9e3779b97f4a7c15;
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
        z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
        return (size_t)(z ^ (z >> 31));
    }
};
using HashSet = unordered_set<int, SplitMix>;

// Standalone hash of a whole sequence, first symbol on BASE^0.
// Use this to hash a pattern you want to search for.
int hash_of(const string &s) {
    int h = 0;
    for (int i = 0; i < (int)s.size(); i++)
        h = add(h, mul(enc((unsigned char)s[i]), pw[i]));
    return h;
}

int hash_of(const vector<int> &v) {
    int h = 0;
    for (int i = 0; i < (int)v.size(); i++)
        h = add(h, mul(enc(v[i]), pw[i]));
    return h;
}

// ----------------------------------------------------------------------------
// Hash — static sequence, O(1) range hash forward and backward.
// NOTE: with `#define int long long` there is only ONE vector constructor.
// vector<int> IS vector<long long>; convert a vector<int32_t> before passing.
// ----------------------------------------------------------------------------
struct Hash {
    int n;
    vector<int> a;    // a[1..n], the raw values
    vector<int> pre;  // pre[i] = unnormalised hash of a[1..i]
    vector<int> suf;  // suf[i] = same, over the reversed array

    Hash() {}

    Hash(const string &s) {
        int m = s.size();
        vector<int> v(m);
        for (int i = 0; i < m; i++) v[i] = (unsigned char)s[i];
        build(v);
    }

    Hash(const vector<int> &v) { build(v); }

    void build(const vector<int> &v) {
        n = v.size();
        a.assign(n + 1, 0);
        pre.assign(n + 1, 0);
        suf.assign(n + 1, 0);
        for (int i = 1; i <= n; i++) a[i] = v[i - 1];
        for (int i = 1; i <= n; i++) {
            pre[i] = add(pre[i - 1], mul(enc(a[i]), pw[i - 1]));
            suf[i] = add(suf[i - 1], mul(enc(a[n + 1 - i]), pw[i - 1]));
        }
    }

    // Subtract the prefix, then divide by BASE^(l-1) to slide the window back
    // so its first symbol sits on BASE^0.
    int get(int l, int r) const {
        if (l > r) return 0;
        return mul(sub(pre[r], pre[l - 1]), ipw[l - 1]);
    }

    // In the reversed array, a[l..r] lives at [n+1-r .. n+1-l], so it is the
    // same formula on suf[].
    int get_rev(int l, int r) const {
        if (l > r) return 0;
        int L = n + 1 - r;
        return mul(sub(suf[n + 1 - l], suf[L - 1]), ipw[L - 1]);
    }

    // Same length AND same hash. The length check is not optional.
    bool equal(int l1, int r1, int l2, int r2) const {
        return r1 - l1 == r2 - l2 && get(l1, r1) == get(l2, r2);
    }

    bool is_pal(int l, int r) const {
        if (l > r) return true;
        return l >= 1 && r <= n && get(l, r) == get_rev(l, r);
    }

    // Longest common prefix of the suffixes starting at i and j.
    // "the first k symbols match" is monotone in k, so bisect on k.
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

    // Longest common suffix of the prefixes ending at i and j.
    int lcs(int i, int j) const {
        if (i == j) return i;
        int lo = 0, hi = min(i, j);
        while (lo < hi) {
            int m = (lo + hi + 1) >> 1;
            if (get(i - m + 1, i) == get(j - m + 1, j)) lo = m;
            else hi = m - 1;
        }
        return lo;
    }

    // Lexicographic compare: -1 if the first range is smaller, 0, or +1.
    // Skip the common prefix in one lcp, then compare the first differing
    // symbol; if one ran out, the shorter one is smaller.
    int compare(int l1, int r1, int l2, int r2) const {
        int len1 = r1 - l1 + 1, len2 = r2 - l2 + 1;
        int cap = min(len1, len2);
        int k = min(lcp(l1, l2), cap);
        if (k == cap) return len1 == len2 ? 0 : (len1 < len2 ? -1 : 1);
        return a[l1 + k] < a[l2 + k] ? -1 : 1;
    }

    // All p < n with prefix(p) == suffix(p).
    vector<int> borders() const {
        vector<int> res;
        for (int p = 1; p < n; p++)
            if (get(1, p) == get(n - p + 1, n)) res.push_back(p);
        return res;
    }

    int longest_border() const {
        for (int p = n - 1; p >= 1; p--)
            if (get(1, p) == get(n - p + 1, n)) return p;
        return 0;
    }

    // Smallest p such that shifting by p leaves the overlap unchanged.
    int period() const {
        for (int p = 1; p <= n; p++)
            if (get(1, n - p) == get(p + 1, n)) return p;
        return n;
    }

    bool has_period(int p) const {
        return p >= n || get(1, n - p) == get(p + 1, n);
    }

    int longest_pal_prefix() const {
        for (int r = n; r >= 1; r--)
            if (is_pal(1, r)) return r;
        return 0;
    }

    int longest_pal_suffix() const {
        for (int l = 1; l <= n; l++)
            if (is_pal(l, n)) return n - l + 1;
        return 0;
    }

    // Every start position where a pattern of length len hashes to h.
    vector<int> occurrences(int h, int len) const {
        vector<int> res;
        for (int i = 1; i + len - 1 <= n; i++)
            if (get(i, i + len - 1) == h) res.push_back(i);
        return res;
    }

    vector<int> occurrences(const string &t) const {
        return occurrences(hash_of(t), t.size());
    }

    // Distinct substrings of a fixed length: dump every window into a set.
    int distinct(int len) const {
        HashSet s;
        for (int i = 1; i + len - 1 <= n; i++) s.insert(get(i, i + len - 1));
        return s.size();
    }

    int distinct_all() const {
        int total = 0;
        for (int len = 1; len <= n; len++) total += distinct(len);
        return total;
    }

    map<int, int> freq(int len) const {
        map<int, int> f;
        for (int i = 1; i + len - 1 <= n; i++) f[get(i, i + len - 1)]++;
        return f;
    }
};

// Longest common extension: how far do A from i and B from j agree?
// Same bisect as Hash::lcp, one endpoint in each sequence.
int lce(const Hash &A, int i, const Hash &B, int j) {
    int lo = 0, hi = min(A.n - i + 1, B.n - j + 1);
    while (lo < hi) {
        int m = (lo + hi + 1) >> 1;
        if (A.get(i, i + m - 1) == B.get(j, j + m - 1)) lo = m;
        else hi = m - 1;
    }
    return lo;
}

// Longest common substring of A and B.
// "a common substring of length L exists" is monotone in L, so bisect on L;
// test one L by hashing all of A's windows into a map and probing B's.
// pa / pb receive the 1-indexed start in A and in B (optional).
int longest_common_substring(const Hash &A, const Hash &B,
                             int *pa = 0, int *pb = 0) {
    int lo = 0, hi = min(A.n, B.n), ra = 1, rb = 1;

    auto feasible = [&](int L, int &x, int &y) {
        if (!L) { x = y = 1; return true; }
        unordered_map<int, int, SplitMix> seen;
        for (int i = 1; i + L - 1 <= A.n; i++)
            seen.emplace(A.get(i, i + L - 1), i);
        for (int j = 1; j + L - 1 <= B.n; j++) {
            auto it = seen.find(B.get(j, j + L - 1));
            if (it != seen.end()) { x = it->second; y = j; return true; }
        }
        return false;
    };

    while (lo < hi) {
        int m = (lo + hi + 1) >> 1, x, y;
        if (feasible(m, x, y)) { lo = m; ra = x; rb = y; }
        else hi = m - 1;
    }
    if (pa) *pa = ra;
    if (pb) *pb = rb;
    return lo;
}

// Longest k with suffix(A, k) == prefix(B, k).
// NOT monotone in k, so binary search here is WRONG. Scanning down with O(1)
// checks is both correct and optimal.
int max_overlap(const Hash &A, const Hash &B) {
    for (int k = min(A.n, B.n); k >= 1; k--)
        if (A.get(A.n - k + 1, A.n) == B.get(1, k)) return k;
    return 0;
}

// Booth: 1-indexed start of the lexicographically least rotation.
// Two candidates i and j race; whichever loses at offset k jumps past k.
// Self-contained — needs nothing else from this file.
int min_rotation(const string &s) {
    int n = s.size(), i = 0, j = 1, k = 0;
    while (i < n && j < n && k < n) {
        char x = s[(i + k) % n], y = s[(j + k) % n];
        if (x == y) { k++; continue; }
        if (x > y) i = i + k + 1;
        else       j = j + k + 1;
        if (i == j) j++;
        k = 0;
    }
    return min(i, j) + 1;
}

// t is a rotation of s iff t occurs inside s+s. Needs MAXN >= 2n.
bool is_rotation(const string &s, const string &t) {
    if (s.size() != t.size()) return false;
    Hash d(s + s);
    return !d.occurrences(t).empty();
}

// Manacher, O(n). Self-contained — needs nothing else from this file.
//   d1[i] = # odd palindromes centred at i (radius including the centre)
//   d2[i] = # even palindromes centred between i-1 and i
void manacher(const string &s, vector<int> &d1, vector<int> &d2) {
    int n = s.size();
    d1.assign(n, 0);
    d2.assign(n, 0);

    for (int i = 0, l = 0, r = -1; i < n; i++) {
        int k = i > r ? 1 : min(d1[l + r - i], r - i + 1);
        while (i - k >= 0 && i + k < n && s[i - k] == s[i + k]) k++;
        d1[i] = k--;
        if (i + k > r) { l = i - k; r = i + k; }
    }

    for (int i = 0, l = 0, r = -1; i < n; i++) {
        int k = i > r ? 0 : min(d2[l + r - i + 1], r - i + 1);
        while (i - k - 1 >= 0 && i + k < n && s[i - k - 1] == s[i + k]) k++;
        d2[i] = k--;
        if (i + k > r) { l = i - k - 1; r = i + k; }
    }
}

// {l, r} of one longest palindromic substring, 1-indexed inclusive.
pair<int, int> longest_pal(const string &s) {
    vector<int> d1, d2;
    manacher(s, d1, d2);
    int bl = 1, br = 1;
    for (int i = 0; i < (int)s.size(); i++) {
        if (2 * d1[i] - 1 > br - bl + 1) { bl = i - d1[i] + 2; br = i + d1[i]; }
        if (d2[i] && 2 * d2[i] > br - bl + 1) { bl = i - d2[i] + 1; br = i + d2[i]; }
    }
    return {bl, br};
}

// Total number of palindromic substrings: each centre contributes its radius.
int count_pal(const string &s) {
    vector<int> d1, d2;
    manacher(s, d1, d2);
    int total = 0;
    for (int i = 0; i < (int)s.size(); i++) total += d1[i] + d2[i];
    return total;
}

int32_t main() {
    fio;
    init_hash(MAXN - 1);   // shrink MAXN when n is small

    return 0;
}
