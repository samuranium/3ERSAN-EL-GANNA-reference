// ============================================================================
// hash_ms.h  —  Zobrist hashing.  ORDER-FREE.  No mod arithmetic, no tables.
// ============================================================================
//
// WHAT THIS FILE DOES
//   Answers questions where ORDER DOES NOT MATTER: "do these two ranges hold
//   the same multiset of values", "is this range a permutation of 1..len",
//   "can this range be rearranged into a palindrome", "which windows of a are
//   anagrams of b", and — via Mo's — "are these two ranges the same up to
//   RELABELLING the values".
//
//   Everything except the last is O(1) per query off a prefix array. No
//   segment tree, no Mo's, no compression.
//
// ----------------------------------------------------------------------------
// TYPES — what must be what, and why
// ----------------------------------------------------------------------------
//   `#define int long long` + `int32_t main()`. Values, indices, counts: all
//   64-bit `int`.
//
//   unsigned long long is REQUIRED for every WEIGHT and every accumulator in
//     this file — `ull` is not decoration here, it is the algorithm:
//       1. splitmix64 needs defined wraparound at 2^64 and a LOGICAL right
//          shift. On a signed type the multiplies are UB and >> sign-extends.
//       2. The multiset hash is a sum of 64-bit weights that DELIBERATELY
//          wraps. Signed overflow is UB; unsigned wraparound is defined and
//          is exactly the group Z/2^64 the argument relies on.
//     So: `ull` for weight_of_value, weight_of_count, all prefix arrays,
//     ms_hash and count_profile. `int` for everything else.
//
//   MEMORY: MoState::cnt is one `int` per compressed value — 8 bytes each.
//     If n and the value range are both ~1e6 and memory is tight, that array
//     (and only that array) is safe to declare `vector<int32_t>`; counts
//     never exceed n.
//
// ----------------------------------------------------------------------------
// MINIMAL BUILDS — when you need X, copy Y
// ----------------------------------------------------------------------------
//   CORE (always, ~15 lines):
//     splitmix, SEED, init_zobrist, weight_of_value
//
//   "same multiset?" on ranges .......... CORE + Zobrist{pre_sum,
//                                         multiset_hash, same_multiset}  ~15
//   "all counts even?" / odd-parity ..... CORE + Zobrist{pre_xor,
//                                         parity_hash, all_even}         ~10
//   "can be a palindrome?" .............. the parity build + present,
//                                         prepare_canpal, can_be_pal     +8
//   "is it a permutation of 1..len?" .... CORE + Zobrist{multiset} +
//                                         PermCheck                      +10
//   anagram windows ..................... CORE + anagram_windows ONLY.
//                                         No Zobrist struct needed.      ~12
//   "same up to relabelling" ............ CORE + weight_of_count +
//                                         compress + MoState + Query +
//                                         mo_run                         ~45
//
//   Only the LAST one needs Mo's. If you reached for Mo's on any of the
//   others, you overbuilt — check this list first.
//
// ----------------------------------------------------------------------------
// DIFFERENT ALGEBRA FROM ROLLING HASH — pick by what the question ignores
// ----------------------------------------------------------------------------
//   sum enc(a[i]) * B^i    position matters          SEQUENCES   (hash.h)
//   sum  weight(a[i])      commutative, invertible   MULTISETS   (here)
//   xor  weight(a[i])      commutative, self-inverse SETS / odd-parity (here)
//
//   Choosing XOR when you needed SUM is the #1 bug in this file: duplicate
//   pairs silently cancel, so {1,1,2} and {2} hash identically.
//   RULE: if the answer changes when an element appears twice, use SUM.
//
// WHY THERE IS NO WEIGHT TABLE
//   weight_of_value(v) is splitmix of v itself, so values up to 1e9 (or any
//   64-bit value) cost zero memory. Only MoState's cnt[] array needs small
//   values — call compress() for that one thing. The hashes never need it.
//
// ----------------------------------------------------------------------------
// QUICK INDEX
// ----------------------------------------------------------------------------
//   init_zobrist()                      once in main; that is the whole setup
//
//   Zobrist z(a);                       1-indexed inclusive
//     z.multiset_hash(l, r)             multiset of values in a[l..r]     O(1)
//     z.parity_hash(l, r)               xor: signature of odd-count values O(1)
//     z.same_multiset(a, b, c, d)       equal multisets?                  O(1)
//     z.all_even(l, r)                  every value an even # of times?   O(1)
//     z.prepare_canpal(a)               call ONCE before can_be_pal
//     z.can_be_pal(l, r)                permutable into a palindrome?     O(1)
//
//   PermCheck pc(a);
//     pc.is_perm(l, r)                  exactly a permutation of 1..len   O(1)
//
//   anagram_windows(a, b)               all starts of anagram windows     O(n)
//   compress(a)                         values -> 1..k               O(n log n)
//   MoState + Query + mo_run            count profile        O((n+q) sqrt n)
//
// TRAPS
//   - SUM for multisets, XOR for sets. Do not swap them.
//   - can_be_pal needs prepare_canpal(a) called once first, or it is wrong.
//   - MoState wants a 1-indexed, COMPRESSED array plus the max value.
// ============================================================================

#include <bits/stdc++.h>
using namespace std;

#define int long long
#define el '\n'
#define ull unsigned long long   // REQUIRED for all weights; see TYPES above
#define fio ios_base::sync_with_stdio(0), cin.tie(0)

// splitmix64: an avalanche mixer. Turns any 64-bit input into something that
// behaves like a random value, which is all Zobrist needs.
// ull is mandatory: wraparound at 2^64 and a logical right shift.
ull splitmix(ull x) {
    x += 0x9e3779b97f4a7c15;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
    x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
    return x ^ (x >> 31);
}

ull SEED;  // per-run salt, so no fixed test can be built against these weights

void init_zobrist() {
    SEED = chrono::steady_clock::now().time_since_epoch().count()
           ^ (ull)(uintptr_t)&SEED;
}

// Random-looking weight of a VALUE. No table, so 1e9 costs nothing.
ull weight_of_value(int v) {
    return splitmix((ull)v ^ SEED);
}

// Random-looking weight of a COUNT. Must map 0 to 0, so a value that
// disappears from a range contributes nothing to the profile.
// The extra odd multiplier keeps count-weights away from value-weights.
ull weight_of_count(int k) {
    return k ? splitmix(((ull)k * 0x100000001b3ull) ^ SEED) : 0;
}

// Map arbitrary values onto 1..k preserving order. Needed ONLY for cnt[].
vector<int> compress(const vector<int> &a) {
    vector<int> vals = a;
    sort(vals.begin(), vals.end());
    vals.erase(unique(vals.begin(), vals.end()), vals.end());
    vector<int> res(a.size());
    for (size_t i = 0; i < a.size(); i++)
        res[i] = lower_bound(vals.begin(), vals.end(), a[i]) - vals.begin() + 1;
    return res;
}

// ----------------------------------------------------------------------------
// Zobrist — static array. O(1) multiset / parity hash of ANY range.
// Both operations form invertible groups, so a prefix array is enough: this
// makes Mo's and segment trees unnecessary for plain "same multiset".
// 1-indexed inclusive; l > r means empty.
// ----------------------------------------------------------------------------
struct Zobrist {
    int n;
    vector<ull> pre_sum;  // prefix sums of weights -> multiset
    vector<ull> pre_xor;  // prefix xors of weights -> odd-count signature

    Zobrist(const vector<int> &a) {
        n = a.size();
        pre_sum.assign(n + 1, 0);
        pre_xor.assign(n + 1, 0);
        for (int i = 1; i <= n; i++) {
            pre_sum[i] = pre_sum[i - 1] + weight_of_value(a[i - 1]);
            pre_xor[i] = pre_xor[i - 1] ^ weight_of_value(a[i - 1]);
        }
    }

    // Plain subtraction: sums are invertible, and duplicates each add their
    // weight again, so counts survive. The wraparound at 2^64 is intended.
    ull multiset_hash(int l, int r) const {
        return l > r ? 0 : pre_sum[r] - pre_sum[l - 1];
    }

    // Pairs cancel under xor, so what survives is exactly the xor of the
    // weights of values occurring an ODD number of times.
    ull parity_hash(int l, int r) const {
        return l > r ? 0 : pre_xor[r] ^ pre_xor[l - 1];
    }

    bool same_multiset(int a, int b, int c, int d) const {
        return b - a == d - c && multiset_hash(a, b) == multiset_hash(c, d);
    }

    // Every value appears an even number of times <=> nothing survives the xor.
    bool all_even(int l, int r) const {
        return parity_hash(l, r) == 0;
    }

    unordered_map<ull, int> present;  // weight -> that value exists in the array

    void prepare_canpal(const vector<int> &a) {
        for (int x : a) present[weight_of_value(x)] = 1;
    }

    // A multiset can be rearranged into a palindrome iff at most one value has
    // an odd count. Zero odd values -> xor is 0. Exactly one -> the xor equals
    // that single value's weight, which is why we look it up in present[].
    bool can_be_pal(int l, int r) const {
        ull s = parity_hash(l, r);
        return !s || present.count(s);
    }
};

// ----------------------------------------------------------------------------
// PermCheck — "is a[l..r] a permutation of 1..(r-l+1)?"
// Compare the range's multiset hash against the precomputed hash of 1..len.
// ----------------------------------------------------------------------------
struct PermCheck {
    Zobrist z;
    vector<ull> ref;  // ref[k] = multiset hash of {1, 2, ..., k}

    PermCheck(const vector<int> &a) : z(a) {
        int n = a.size();
        ref.assign(n + 2, 0);
        for (int i = 1; i <= n + 1; i++) ref[i] = ref[i - 1] + weight_of_value(i);
    }

    bool is_perm(int l, int r) const {
        return z.multiset_hash(l, r) == ref[r - l + 1];
    }
};

// All 1-indexed starts where a window of a is an anagram of b.
// Sliding window on the multiset sum: add the entering element, remove the
// leaving one, compare against the target. O(1) per step, self-contained.
vector<int> anagram_windows(const vector<int> &a, const vector<int> &b) {
    int n = a.size(), m = b.size();
    vector<int> res;
    if (!m || m > n) return res;

    ull need = 0, cur = 0;
    for (int x : b) need += weight_of_value(x);

    for (int i = 0; i < n; i++) {
        cur += weight_of_value(a[i]);
        if (i >= m) cur -= weight_of_value(a[i - m]);
        if (i >= m - 1 && cur == need) res.push_back(i - m + 2);
    }
    return res;
}

// ----------------------------------------------------------------------------
// MoState — pointer-move state for Mo's algorithm.
//   ms_hash        multiset of values   (prefix-doable; here for free)
//   count_profile  multiset of COUNTS   (the ONLY reason Mo's is needed)
//   distinct       number of distinct values
// Feed it a 1-indexed COMPRESSED array plus the largest compressed value.
// ----------------------------------------------------------------------------
struct MoState {
    vector<int> a;
    vector<int> cnt;      // int32_t is enough here if memory is tight
    ull ms_hash = 0;
    ull count_profile = 0;
    int distinct = 0;

    MoState(const vector<int> &arr, int max_val) : a(arr), cnt(max_val + 2, 0) {}

    // Retract the old count's contribution, bump, reinsert the new one.
    // The count profile is NOT a sum over elements, which is why no prefix
    // trick exists for it and why this struct has to exist at all.
    void insert(int i) {
        int v = a[i];
        count_profile -= weight_of_count(cnt[v]);
        ms_hash += weight_of_value(v);
        if (!cnt[v]++) distinct++;
        count_profile += weight_of_count(cnt[v]);
    }

    void erase(int i) {
        int v = a[i];
        count_profile -= weight_of_count(cnt[v]);
        ms_hash -= weight_of_value(v);
        if (!--cnt[v]) distinct--;
        count_profile += weight_of_count(cnt[v]);
    }
};

struct Query {
    int l, r, id;
};

// Mo's driver. emit(query, state) is called once per query with the state
// already positioned on [l, r]. Every pointer move is O(1).
// Odd-even block ordering: on odd blocks the right pointer walks backwards,
// which halves total travel.
template <class F>
void mo_run(int n, vector<Query> &qs, MoState &state, F emit) {
    int block = max<int>(1, n / max(1.0, sqrt((double)qs.size())));

    sort(qs.begin(), qs.end(), [&](const Query &x, const Query &y) {
        int bx = x.l / block, by = y.l / block;
        if (bx != by) return bx < by;
        return (bx & 1) ? x.r > y.r : x.r < y.r;
    });

    int L = 1, R = 0;
    for (auto &q : qs) {
        while (R < q.r) state.insert(++R);
        while (L > q.l) state.insert(--L);
        while (R > q.r) state.erase(R--);
        while (L < q.l) state.erase(L++);
        emit(q, state);
    }
}

int32_t main() {
    fio;
    init_zobrist();

    return 0;
}
