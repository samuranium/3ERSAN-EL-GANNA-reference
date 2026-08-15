// ============================================================================
// PERSISTENT BINARY TRIE -- max XOR over a RANGE, not just the whole array
// ----------------------------------------------------------------------------
// DS/Trie/binary_trie.cpp answers "max a[i] xor x" over everything inserted.
// The moment the question is restricted to a subarray, you need one trie
// VERSION per prefix, and the range trie is the difference of two versions:
//
//     count in [l, r] at a node  =  cnt[root_r][node] - cnt[root_{l-1}][node]
//
// Insert copies only the path (B+1 nodes), so B+1 nodes per insert and the
// whole structure is O(n * B).
//
// The classic use: prefix XOR. `pre[i] = a[1] ^ ... ^ a[i]`, and the XOR of
// the subarray (l, r] is `pre[r] ^ pre[l-1]`. So "max XOR subarray ending at
// r with left end >= L" is "max over i in [L-1, r-1] of pre[r] ^ pre[i]" --
// exactly a range query on this structure.
//
// ############################################################################
// #  THE SUBTRACTION IS ON THE COUNT, AND BOTH VERSIONS MUST BE WALKED IN LOCKSTEP
// #
// #  Descending, you hold TWO node ids -- one from version r, one from
// #  version l-1 -- and step both the same way. A child "exists in the range"
// #  iff  cnt[child_r] - cnt[child_l] > 0,  NOT  child_r != 0.
// #
// #  Using node existence instead of the count difference silently includes
// #  values inserted BEFORE l. Every test where the answer happens to lie
// #  inside the range still passes.
// ############################################################################
//
// ############################################################################
// #  root[0] MUST BE A REAL EMPTY TRIE, NOT 0
// #
// #  Version 0 is "nothing inserted". If you leave root[0] = 0 and 0 is also
// #  the null node, the lockstep descent reads cnt[0] for the left version --
// #  which happens to work, but only because cnt[0] == 0. Build an explicit
// #  empty root anyway: the moment you add a lazy field or an aggregate other
// #  than a count, the accidental version breaks.
// ############################################################################
//
// PITFALLS:
//   Node pool: n * (B + 1) + 1. n = 2e5 with B = 30 is 6.2e6 -- size it from
//     that product, not from a guess.
//   B must cover the largest value: values up to 1e9 need B = 30 (bits 0..29);
//     up to 1e18 need B = 60 and ll everywhere.
//   Insert version i FROM version i-1. Building each from scratch is O(n^2).
//   The prefix-XOR framing shifts the index by one. Insert pre[0] = 0 as
//     version 1 before any array element, or subarrays starting at index 1
//     are unreachable.
//   For MINIMUM xor, follow the SAME bit instead of the opposite one.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   max (x xor a[i]) over i in [l, r]              max_xor()
//   min (x xor a[i]) over i in [l, r]              same descent, same-bit
//                                                  preference
//   max XOR SUBARRAY within [l, r]                 prefix XOR + this
//   k-th largest (x xor a[i]) in a range           descend carrying k against
//                                                  the count difference
//   count of i in [l,r] with (x xor a[i]) < K      walk K's bits; when K's bit
//                                                  is 1, add the whole subtree
//                                                  that keeps the xor bit 0
//   count pairs with xor < K                       the above, summed over i
//   max XOR over a TREE PATH                       version per root-to-node
//                                                  path, subtract at the LCA
//                                                  (twice, plus the LCA value)
//   max xor over the whole array                   plain binary trie --
//                                                  DS/Trie/binary_trie.cpp
//   max xor of a SUBSET                            xor basis, not a trie --
//                                                  DP/sub-problems xorbasis.cpp
//
// NOT THIS:
//   the range is always the whole array -> plain binary trie, no versions.
//   you need a SUBSET xor              -> linear basis.
//   values change                      -> persistence is append-only; use a
//     BIT-of-tries or offline processing.
// ============================================================================
const int B = 30;                       // bits 0..B-1; values < 2^B
const int POOL = 200005 * (B + 1);

int ch[POOL][2], cnt[POOL];
int tot;
int root[200005];

int new_node(int from) {                // copy-on-write
    int v = ++tot;
    ch[v][0] = ch[from][0], ch[v][1] = ch[from][1];
    cnt[v] = cnt[from];
    return v;
}
void init() {
    tot = 0;
    ch[0][0] = ch[0][1] = 0, cnt[0] = 0;
    root[0] = new_node(0);              // an explicit EMPTY version 0
}
// version `prev` plus the value x -> a new version
int insert(int prev, int x) {
    int cur = new_node(prev), res = cur;
    cnt[cur]++;
    for (int b = B - 1; b >= 0; b--) {
        int d = x >> b & 1;
        ch[cur][d] = new_node(ch[cur][d]);
        cur = ch[cur][d];
        cnt[cur]++;
    }
    return res;
}
// max (x xor v) over the values inserted in versions (lv, rv]
int max_xor(int lv, int rv, int x) {
    int a = lv, b = rv, res = 0;
    for (int i = B - 1; i >= 0; i--) {
        int d = x >> i & 1, want = d ^ 1;                 // prefer the opposite
        int ca = ch[a][want], cb = ch[b][want];
        if (cnt[cb] - cnt[ca] > 0) {                      // COUNT, not existence
            res |= 1 << i;
            a = ca, b = cb;
        } else {
            a = ch[a][d], b = ch[b][d];
        }
    }
    return res;
}
// min (x xor v) over the same range: follow the SAME bit when possible
int min_xor(int lv, int rv, int x) {
    int a = lv, b = rv, res = 0;
    for (int i = B - 1; i >= 0; i--) {
        int d = x >> i & 1;
        int ca = ch[a][d], cb = ch[b][d];
        if (cnt[cb] - cnt[ca] > 0) {
            a = ca, b = cb;
        } else {
            res |= 1 << i;
            a = ch[a][d ^ 1], b = ch[b][d ^ 1];
        }
    }
    return res;
}
////////////////////////////////////////////////////////////////////////////////
// MAX XOR SUBARRAY -- the version indexing, written out, because it is the
// part that is easy to get wrong (and looks correct on many tests).
//
//   pre[0] = 0, pre[i] = a[1]^...^a[i]
//   ver[k] holds pre[0 .. k-1];  ver[0] is the EMPTY version.
//       for (int i = 0; i <= n; i++) ver[i+1] = insert(ver[i], pre[i]);
//
//   max xor of a subarray ending at r, any start:
//       max_xor(ver[0],   ver[r], pre[r])
//   ... with the left end at index >= L:
//       max_xor(ver[L-1], ver[r], pre[r])
//
// Note ver[r], not ver[r+1]: the candidates are pre[0..r-1], since the
// subarray (l-1, r] uses pre[l-1] with l-1 <= r-1.
////////////////////////////////////////////////////////////////////////////////

// how many v in the range have (x xor v) < K
int count_xor_less(int lv, int rv, int x, int K) {
    int a = lv, b = rv, res = 0;
    for (int i = B - 1; i >= 0; i--) {
        if (cnt[b] - cnt[a] == 0) break;
        int d = x >> i & 1, k = K >> i & 1;
        if (k) {                        // xor-bit 0 keeps us strictly below
            res += cnt[ch[b][d]] - cnt[ch[a][d]];
            a = ch[a][d ^ 1], b = ch[b][d ^ 1];
        } else {
            a = ch[a][d], b = ch[b][d];
        }
    }
    return res;
}
