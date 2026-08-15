// ============================================================================
// RANDOMIZATION -- when randomness is the algorithm, not the input
// ----------------------------------------------------------------------------
// The MECHANICS (mt19937 seeding, uniform_int_distribution, shuffle) are in the
// other team's Misc sheet. This file is the part that decides problems: WHEN
// randomising turns something hard into something easy, and what failure
// probability you are accepting.
//
// ############################################################################
// #  SEED FROM THE CLOCK, NEVER FROM A CONSTANT
// #
// #      mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());
// #
// #  A fixed seed is deterministic, so an anti-test written against your
// #  submission works every time. This matters on Codeforces open hacking and
// #  on any judge with rejudging.
// #  rand() is worse still: 15-bit RAND_MAX on MinGW, and a weak generator.
// ############################################################################
//
// ############################################################################
// #  MONTE CARLO vs LAS VEGAS -- know which one you shipped
// #
// #    Monte Carlo   always fast, sometimes WRONG.  Choose the repeat count so
// #                  the error probability is negligible.
// #    Las Vegas     always right, sometimes SLOW.  Expected time is the claim.
// #
// #  k independent trials each wrong with probability p give error p^k.
// #  p = 1/2, k = 60  ->  1e-18. That is the usual budget: 30-60 rounds.
// ############################################################################
//
// ----------------------------------------------------------------------------
// WHERE RANDOMNESS IS THE ANSWER
// ----------------------------------------------------------------------------
// [1] VERIFY A PRODUCT WITHOUT COMPUTING IT -- Freivalds, below.
//     "Is A*B == C" in O(n^2) instead of O(n^3). Multiply by a random 0/1
//     vector: A(Bx) vs Cx. If A*B != C the test fails with probability >= 1/2
//     per trial, so 30 trials is certainty in practice.
//
// [2] KILL THE ADVERSARIAL CASE by shuffling.
//     Quicksort pivots, unordered_map insertion order, "sorted input" traps.
//     shuffle(v.begin(), v.end(), rng) before sorting or inserting removes the
//     worst case entirely. Also the fix for anti-quicksort tests.
//     unordered_map is attackable on Codeforces -- use a splitmix64 custom hash
//     with a clock-seeded salt, or just use map.
//
// [3] SALT YOUR HASHES. A fixed base is what anti-hash tests target. Random
//     base per run makes a targeted collision impossible.
//     hashing/hash.cpp does this; hashing/hash_ms.cpp is Zobrist, which IS
//     this idea taken all the way.
//
// [4] RANDOM SAMPLING FOR MAJORITY / HEAVY ELEMENTS.
//     "Does some value occupy more than half the range?" Sample 30 positions
//     and test each properly. A majority element is picked with probability
//     1 - 2^-30. Generalises: an element with frequency > 1/k needs ~k*30
//     samples. Turns many "range majority" problems into a check plus a
//     frequency structure.
//
// [5] BREAK TIES TO MAKE A SOLUTION UNIQUE. Add tiny random weights and the
//     minimum becomes unique with high probability (isolation lemma). Useful
//     when an algorithm needs "the unique minimum" to be well defined.
//
// [6] RANDOM PIVOT / RANDOM ORDER makes many expected bounds work:
//     nth_element is O(n) expected, treap priorities are random on purpose
//     (DS/bst/treap_implicit.cpp), and randomized incremental construction is
//     how smallest-enclosing-circle gets to O(n).
//
// [7] BIRTHDAY BOUND. Among k random values from a space of size N, a
//     collision appears once k ~ sqrt(N). Two consequences:
//       - 64-bit hashes collide at ~2^32 items. Fine for n = 1e6, not for a
//         structure that hashes 1e10 things.
//       - Pollard's rho and BSGS both live on this bound.
//
// [8] WHEN NOTHING ELSE FITS: random restarts / simulated annealing for
//     NP-hard optimisation with a generous time limit. Last resort, but it
//     scores on problems where nothing exact fits.
//
// PITFALLS:
//   rng() % k is BIASED when k does not divide 2^64. It rarely matters for
//     generating tests; it matters when the argument depends on uniformity.
//     Use uniform_int_distribution then.
//   "Random" tests are not adversarial tests. See stress/gen.cpp -- a uniform
//     random tree will not find your stack overflow.
//   Do not reseed inside a loop. Construct the generator once, globally.
// ============================================================================
mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());

ll rnd(ll l, ll r) { return uniform_int_distribution<ll>(l, r)(rng); }

// [1] FREIVALDS -- is A*B == C? O(n^2) per trial, error <= 2^-trials.
// Returns false only if it is CERTAIN they differ; true means "probably equal".
bool freivalds(const vector<vector<ll>> &A, const vector<vector<ll>> &B,
               const vector<vector<ll>> &C, int n, ll mod, int trials = 30) {
    for (int t = 0; t < trials; t++) {
        vector<ll> x(n), Bx(n, 0), ABx(n, 0), Cx(n, 0);
        for (int i = 0; i < n; i++) x[i] = rnd(0, 1);
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++) Bx[i] = (Bx[i] + B[i][j] * x[j]) % mod;
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++) ABx[i] = (ABx[i] + A[i][j] * Bx[j]) % mod;
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++) Cx[i] = (Cx[i] + C[i][j] * x[j]) % mod;
        for (int i = 0; i < n; i++)
            if (((ABx[i] - Cx[i]) % mod + mod) % mod) return false;
    }
    return true;
}
// [4] sample-based majority: is there a value covering more than half of v?
// Returns it, or -1. Wrong with probability <= 2^-trials when one exists.
ll sampled_majority(const vector<ll> &v, int trials = 30) {
    int n = v.size();
    if (!n) return -1;
    for (int t = 0; t < trials; t++) {
        ll cand = v[rnd(0, n - 1)];
        int c = 0;
        for (ll x : v) c += (x == cand);
        if (c * 2 > n) return cand;
    }
    return -1;
}
// [2] custom hash for unordered_map / gp_hash_table -- defeats anti-hash tests
struct SafeHash {
    static uint64_t splitmix64(uint64_t x) {
        x += 0x9e3779b97f4a7c15ULL;
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
        x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
        return x ^ (x >> 31);
    }
    size_t operator()(uint64_t x) const {
        static const uint64_t SALT =
            chrono::steady_clock::now().time_since_epoch().count();
        return splitmix64(x + SALT);
    }
};
