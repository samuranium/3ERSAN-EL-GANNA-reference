// ============================================================================
// CATALAN NUMBERS -- C(n) = 1, 1, 2, 5, 14, 42, 132, 429, 1430, 4862, ...
// ----------------------------------------------------------------------------
//   C(n) = binom(2n, n) / (n+1)  =  binom(2n, n) - binom(2n, n+1)
//   C(n) = sum over i of C(i)*C(n-1-i)          (split at the matching close)
//   C(n) = C(n-1) * 2(2n-1) / (n+1)             (one step, no factorials)
//
// ############################################################################
// #  RECOGNISING IT -- the shape behind every application below
// #
// #  Count sequences of n "+1" and n "-1" whose every PREFIX SUM stays >= 0.
// #
// #  Open/close, push/pop, up/right, left-child/right-child -- they are all
// #  that one statement. If you can phrase your objects as a walk that must
// #  never go below zero and ends at zero, the answer is C(n).
// #
// #  The proof is the reflection argument, and it is also the formula:
// #  binom(2n,n) total walks minus binom(2n,n+1) bad ones.
// ############################################################################
//
// ############################################################################
// #  THE OFF-BY-ONE THAT COSTS THE MOST
// #
// #  n TRIANGLES from a polygon needs C(n) but the polygon has n+2 VERTICES.
// #  n INTERNAL NODES in a binary tree gives C(n) shapes, but n NODES TOTAL
// #  also gives C(n) -- different trees, same count, different n.
// #  Always fix WHICH quantity is n by checking a tiny case by hand:
// #  a square (4 vertices) has 2 triangulations, and C(2) = 2.
// ############################################################################
//
// ----------------------------------------------------------------------------
// APPLICATIONS -- all of these are C(n), and all are the same problem
// ----------------------------------------------------------------------------
//   balanced bracket sequences of length 2n
//   ways to fully parenthesise n+1 factors
//   rooted binary trees with n internal nodes (or n nodes -- see the box)
//   triangulations of a convex polygon with n+2 vertices
//   monotone lattice paths from (0,0) to (n,n) staying weakly below y = x
//   permutations of 1..n that are STACK-SORTABLE
//   permutations avoiding any ONE pattern of length 3 (231, 132, ...)
//   non-crossing partitions of an n-set
//   non-crossing chord pairings of 2n points on a circle
//   Dyck paths of semilength n
//   ways to cut a staircase of height n into n rectangles
//   rooted plane trees with n+1 vertices
//   sequences of n pushes and n pops of a stack, never popping empty
//
// ----------------------------------------------------------------------------
// NEIGHBOURS -- when it is ALMOST Catalan
// ----------------------------------------------------------------------------
//   paths that may touch but not cross, from (0,0) to (a,b), staying below
//     the diagonal          -> BALLOT / cycle lemma:  (a-b)/(a+b) * binom(a+b, a)
//   paths from (0,0) to (n,n) that must stay strictly below except at the
//     ends                  -> C(n-1)
//   paths in a grid avoiding a line y = x + k
//                           -> reflection again: binom(total, up) - binom(total, up - k - 1)
//   steps of +1, -1 AND 0 with the same non-negativity condition
//                           -> MOTZKIN numbers
//   Dyck paths with exactly k peaks
//                           -> NARAYANA:  binom(n,k)*binom(n,k-1)/n
//   binary trees counted by shape AND labels
//                           -> C(n) * n!
//   partitions into k non-crossing blocks
//                           -> Narayana again
//   "at least as many A as B at every prefix", |A| != |B|
//                           -> the ballot formula, not C(n)
//
// PITFALLS:
//   C(n) grows like 4^n / n^1.5. C(35) already exceeds 2^63 -- work mod p
//     unless n is tiny.
//   The division by (n+1) is modular: multiply by the inverse, never integer-
//     divide. Needs a prime modulus -- math/modular.cpp.
//   The convolution recurrence is O(n^2); use it only when you need every
//     value AND the modulus is awkward. The closed form is O(1) per value
//     after factorials.
// ============================================================================
// needs mod / add / mult / fp and factorials -- combinatorics/nCr_with_mod.cpp

// C(n) from factorials, O(1) each once fac/ifac are built
int catalan(int n) {
    return mult(ncr(2 * n, n), fp(n + 1, mod - 2));
}
// every C(0..n) in O(n) with one inverse per step, no factorials needed
vector<int> catalan_table(int n) {
    vector<int> c(n + 1);
    c[0] = 1;
    for (int i = 0; i < n; i++)                    // C(i+1) = C(i)*2(2i+1)/(i+2)
        c[i + 1] = mult(mult(c[i], 2 * (2 * i + 1) % mod), fp(i + 2, mod - 2));
    return c;
}
// the convolution form -- O(n^2), but assumes nothing about the modulus
vector<int> catalan_conv(int n) {
    vector<int> c(n + 1, 0);
    c[0] = 1;
    for (int i = 1; i <= n; i++)
        for (int j = 0; j < i; j++)
            c[i] = add(c[i], mult(c[j], c[i - 1 - j]));
    return c;
}
// BALLOT / cycle lemma: paths to (a,b) with a >= b that stay strictly ahead
int ballot(int a, int b) {
    if (a < b) return 0;
    return mult(mult(a - b, fp(a + b, mod - 2)), ncr(a + b, a));
}
// NARAYANA N(n,k): Dyck paths of semilength n with exactly k peaks; sums to C(n)
int narayana(int n, int k) {
    if (k < 1 || k > n) return 0;
    return mult(mult(ncr(n, k), ncr(n, k - 1)), fp(n, mod - 2));
}
