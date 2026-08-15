// ============================================================================
// SOS DP (sum over subsets / zeta transform) -- O(2^N * N)
// ----------------------------------------------------------------------------
//   F[mask] = sum of A[sub] over every sub that is a SUBMASK of mask
//
// The naive version is O(3^N) by enumerating submasks. This is O(2^N * N):
// fix one bit at a time and merge the two halves. Think of it as an
// N-DIMENSIONAL PREFIX SUM over a 2x2x...x2 grid -- each bit is one axis, and
// the outer loop is "prefix-sum along axis i". That framing is the whole idea,
// and it is why the loop order below is bit-outer, mask-inner.
//
// ############################################################################
// #  THE BIT LOOP MUST BE OUTSIDE
// #
// #      for i in bits:  for mask:  if (mask>>i&1) F[mask] += F[mask^(1<<i)];
// #
// #  Swapping the loops gives a partial, order-dependent answer that looks
// #  plausible on small N. Same rule as a multidimensional prefix sum: finish
// #  one axis before starting the next.
// ############################################################################
//
// ############################################################################
// #  ZETA vs MOBIUS -- the transform and its INVERSE
// #
// #    forward (subset sums):   F[mask] += F[mask ^ (1<<i)]
// #    inverse (recover A):     F[mask] -= F[mask ^ (1<<i)]
// #
// #  Identical loops, one sign. The inverse is what turns "at least" counts
// #  into "exactly" counts -- the subset analogue of DP/incexc.cpp.
// ############################################################################
//
// SUPERSET version (sum over all masks CONTAINING mask): flip the test to
//   if (!(mask>>i&1)) F[mask] += F[mask | (1<<i)];
//
// PITFALLS:
//   N is the number of BITS, not the number of masks. Arrays are size 1<<N.
//   Overflow: 2^N terms are summed into one cell -- use ll or a modulus.
//   The first block below keeps a per-bit layer (dp[mask][i]) for clarity;
//     the second is the same thing with the layer dropped. Use the second.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   for each mask, aggregate over its SUBMASKS       this
//   for each mask, aggregate over its SUPERMASKS     the flipped version above
//   count pairs (i,j) with a[i] & a[j] == 0          SOS over complements:
//                                                    for each x, how many y
//                                                    are submasks of ~x
//   count pairs with a[i] | a[j] == full             same, complemented
//   count pairs with a[i] & a[j] == k                SOS then Mobius on k
//   number of subsets whose OR is exactly S          zeta, then inverse
//   max/min over submasks instead of sum             replace += with max/min;
//                                                    idempotent, so no inverse
//   "for each mask, the best two submask values"     carry a pair, merge both
//   subset-sum convolution                           SOS with a popcount layer
//                                                    (ranked zeta transform)
//   xor / and / or CONVOLUTION                       different tool -- FWHT,
//                                                    math/fwht.cpp
//   multidimensional prefix sums over a real grid    same loop shape, more
//                                                    values per axis --
//                                                    DS/prefix_sum_2d.cpp
//
// NOT THIS: aggregating over submasks of ONE mask is just the submask loop,
//   O(2^popcount). SOS pays off when you need the answer for EVERY mask.
// ============================================================================
//iterative version
for(int mask = 0; mask < (1<<N); ++mask){
	dp[mask][-1] = A[mask];	//handle base case separately (leaf states)
	for(int i = 0;i < N; ++i){
		if(mask & (1<<i))
			dp[mask][i] = dp[mask][i-1] + dp[mask^(1<<i)][i-1];
		else
			dp[mask][i] = dp[mask][i-1];
	}
	F[mask] = dp[mask][N-1];
}
//memory optimized, super easy to code.
for(int i = 0; i<(1<<N); ++i)
	F[i] = A[i];
for(int i = 0;i < N; ++i) for(int mask = 0; mask < (1<<N); ++mask){
	if(mask & (1<<i))
		F[mask] += F[mask^(1<<i)];
}