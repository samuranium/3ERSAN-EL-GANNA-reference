// ============================================================================
// BURNSIDE / POLYA -- counting up to rotation, reflection, relabelling
// ----------------------------------------------------------------------------
// BURNSIDE'S LEMMA: the number of distinct objects under a symmetry group G is
// the AVERAGE number of objects each group element leaves unchanged:
//
//     answer = (1 / |G|) * sum over g in G of Fix(g)
//
// The whole job is computing Fix(g). When colours are UNLIMITED that is easy:
// a colouring is fixed by g exactly when every CYCLE of g is monochromatic, so
//
//     Fix(g) = c ^ (number of cycles of g)
//
// ############################################################################
// #  c^cycles IS ONLY VALID WHEN EVERY COLOUR IS UNLIMITED
// #
// #  "How many necklaces with n beads and c colours" -> c^cycles is right.
// #  "How many with EXACTLY 3 red and 2 blue"        -> it is WRONG.
// #
// #  With fixed colour counts you need the POLYA cycle index: each cycle of
// #  length L consumes L beads of one colour, so Fix(g) is a knapsack over
// #  the cycle lengths. polya_fixed_counts() below does exactly that.
// #  Using c^cycles here gives a number that is far too large and looks
// #  plausible.
// ############################################################################
//
// ############################################################################
// #  GET THE GROUP RIGHT, AND ALWAYS INCLUDE THE IDENTITY
// #
// #  rotations only            CYCLIC group, n elements
// #  rotations AND reflections DIHEDRAL group, 2n elements
// #
// #  The identity contributes c^n and it is easy to forget. Reflections split
// #  by parity:
// #      n ODD  : n axes, each through one bead -> (n+1)/2 cycles
// #      n EVEN : n/2 axes through two beads    -> n/2 + 1 cycles
// #               n/2 axes through no bead      -> n/2 cycles
// #  Using one reflection formula for both parities is the classic wrong
// #  answer, and it is correct for exactly half the inputs.
// ############################################################################
//
// PITFALLS:
//   The division by |G| is MODULAR -- multiply by the inverse. |G| is small
//     and coprime to a prime modulus, so this is safe.
//   A rotation by k on n beads has exactly gcd(n, k) cycles, each of length
//     n/gcd(n,k). That one fact collapses the cyclic case to a divisor sum.
//   For n up to 1e9 you cannot enumerate rotations; use the phi form:
//     necklaces = (1/n) * sum over d | n of phi(d) * c^(n/d).
//   Burnside counts EQUIVALENCE CLASSES. If the problem asks for a
//     representative or for the class SIZES, this gives neither.
//   Reflections of a 2D grid / cube: enumerate the group explicitly and count
//     cycles by union-find. Do not guess the cycle structure.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   necklaces: n beads, c colours, up to ROTATION       necklaces()
//   ... up to rotation AND reflection (bracelets)        bracelets()
//   ... with EXACTLY k_i beads of colour i               polya_fixed_counts()
//   colour a cycle / ring / round table                  same as necklaces
//   count distinct grids up to 90-degree rotation        enumerate the 4 maps,
//                                                        count cycles by DSU,
//                                                        burnside_explicit()
//   ... up to rotation + reflection (8 symmetries)       same, 8 maps
//   count distinct graphs up to vertex relabelling       group = S_n; only
//                                                        feasible for tiny n
//   "two arrangements are the same if one is a           necklaces / bracelets
//    rotation of the other"
//   count binary strings up to rotation                  necklaces(n, 2)
//   distinct dice / cube colourings                      burnside_explicit()
//                                                        with the 24 rotations
//
// NOT THIS:
//   "the number of distinct ROTATIONS of one given string" -> n / smallest
//     period. That is a string question, not a Burnside one -- strings/lyndon.cpp.
//   the group is huge and unstructured -> Burnside needs you to enumerate G
//     or to know its cycle statistics.
// ============================================================================
const ll MOD = 1000000007;

ll pw(ll b, ll e, ll m = MOD) {
    ll r = 1; b %= m;
    while (e) { if (e & 1) r = r * b % m; b = b * b % m, e >>= 1; }
    return r;
}
ll inv(ll a) { return pw(a, MOD - 2); }

ll phi(ll x) {                              // Euler totient of one number
    ll r = x;
    for (ll p = 2; p * p <= x; p++)
        if (x % p == 0) { while (x % p == 0) x /= p; r -= r / p; }
    if (x > 1) r -= r / x;
    return r;
}

// ---- necklaces: n beads, c colours, rotations only ------------------------
// (1/n) * sum over d | n of phi(d) * c^(n/d).   Works for n up to ~1e18.
ll necklaces(ll n, ll c) {
    ll total = 0;
    for (ll d = 1; d * d <= n; d++) {
        if (n % d) continue;
        total = (total + phi(d) % MOD * pw(c, n / d)) % MOD;
        if (d != n / d) total = (total + phi(n / d) % MOD * pw(c, d)) % MOD;
    }
    return total % MOD * inv(n % MOD) % MOD;
}

// ---- bracelets: rotations AND reflections. The parity split is in the box.
ll bracelets(ll n, ll c) {
    ll rot = 0;
    for (ll d = 1; d * d <= n; d++) {
        if (n % d) continue;
        rot = (rot + phi(d) % MOD * pw(c, n / d)) % MOD;
        if (d != n / d) rot = (rot + phi(n / d) % MOD * pw(c, d)) % MOD;
    }
    ll refl;
    if (n & 1) refl = n % MOD * pw(c, (n + 1) / 2) % MOD;         // n odd
    else refl = (n / 2 % MOD * pw(c, n / 2 + 1) % MOD
               + n / 2 % MOD * pw(c, n / 2) % MOD) % MOD;         // n even
    return (rot + refl) % MOD * inv(2 * (n % MOD) % MOD) % MOD;
}

////////////////////////////////////////////////////////////////////////////////
// POLYA WITH FIXED COLOUR COUNTS
//
// Each cycle of length L must be a single colour, consuming L beads of it.
// Fix(g) is therefore the number of ways to assign whole cycles to colours so
// that colour i receives exactly cnt[i] beads -- a multi-dimensional knapsack
// over the cycle lengths.
//
// Below: rotations only, colour counts in cnt[]. n must be small enough for
// the knapsack (n <= ~50 with 2-3 colours, or use the multinomial shortcut
// when all cycles have the SAME length, which they do for a pure rotation).
//
// For a rotation by k, every cycle has length L = n/gcd(n,k) and there are
// g = gcd(n,k) of them. So Fix is 0 unless L divides every cnt[i], and
// otherwise it is the multinomial (g choose cnt[0]/L, cnt[1]/L, ...).
////////////////////////////////////////////////////////////////////////////////
ll fact_[1000005], ifact_[1000005];
void init_fact(int n) {
    fact_[0] = 1;
    for (int i = 1; i <= n; i++) fact_[i] = fact_[i - 1] * i % MOD;
    ifact_[n] = pw(fact_[n], MOD - 2);
    for (int i = n; i >= 1; i--) ifact_[i - 1] = ifact_[i] * i % MOD;
}

// necklaces with exactly cnt[i] beads of colour i, up to rotation
ll polya_fixed_counts(int n, vector<int> cnt) {
    ll total = 0;
    for (int k = 0; k < n; k++) {
        int g = __gcd(n, k == 0 ? n : k);        // cycles; k=0 is the identity
        int L = n / g;                           // each cycle has this length
        bool ok = true;
        ll f = fact_[g];
        for (int x : cnt) {
            if (x % L) { ok = false; break; }
            f = f * ifact_[x / L] % MOD;
        }
        if (ok) total = (total + f) % MOD;       // the multinomial
    }
    return total * inv(n) % MOD;
}

////////////////////////////////////////////////////////////////////////////////
// EXPLICIT GROUP -- when you cannot reason about the cycle structure.
//
// Give each group element as a permutation of the m positions. Count its
// cycles directly, sum c^cycles, divide by |G|. This is the version to use for
// grids, cubes, and anything where guessing the cycle count is a coin flip.
////////////////////////////////////////////////////////////////////////////////
ll burnside_explicit(vector<vector<int>> &G, ll c) {
    int m = G[0].size();
    ll total = 0;
    for (auto &g : G) {
        vector<int> vis(m, 0);
        int cycles = 0;
        for (int i = 0; i < m; i++)
            if (!vis[i]) {
                cycles++;
                for (int j = i; !vis[j]; j = g[j]) vis[j] = 1;
            }
        total = (total + pw(c, cycles)) % MOD;
    }
    return total * inv(G.size()) % MOD;
}
