// ============================================================================
// LYNDON FACTORISATION (Duval) + MINIMAL ROTATION -- O(n) time, O(1) space
// ----------------------------------------------------------------------------
// A LYNDON WORD is a string strictly smaller than all of its proper rotations
// (equivalently: strictly smaller than all of its proper suffixes).
//
// CHEN-FOX-LYNDON: every string factors UNIQUELY as
//     s = w1 w2 ... wk    with each wi a Lyndon word and w1 >= w2 >= ... >= wk
// Duval's algorithm produces it in O(n) with two pointers and no memory.
//
// Two payoffs that come free from the factorisation:
//     the LAST factor is the lexicographically smallest SUFFIX of s
//     Duval run on s+s, stopped at n, gives the MINIMAL ROTATION
//
// ############################################################################
// #  THE THREE-POINTER INVARIANT IS THE WHOLE ALGORITHM
// #
// #      i  start of the current block of factors
// #      j  scanning pointer
// #      k  comparison pointer, always j - (current period)
// #
// #      s[j] >  s[k]   the block is a Lyndon prefix, restart k at i
// #      s[j] == s[k]   the period continues, advance both
// #      s[j] <  s[k]   emit floor blocks of length (j - k), then rewind
// #
// #  The emit loop is `while (i <= k) { output i .. i + (j-k) - 1;
// #  i += j - k; }` -- it emits SEVERAL equal factors at once. Emitting one
// #  and looping is the classic off-by-one that drops repeats.
// ############################################################################
//
// ############################################################################
// #  MINIMAL ROTATION NEEDS s+s AND AN EARLY STOP
// #
// #  Run Duval over the doubled string and take the start of the factor that
// #  begins at index < n and is the smallest such start. Practically: track
// #  the best start while factoring, and BREAK as soon as i >= n. Without the
// #  break you scan 2n and can return a start in the second copy.
// #
// #  Booth's algorithm does the same job with a failure function. Duval is
// #  shorter and needs no array -- prefer it.
// ############################################################################
//
// PITFALLS:
//   Lyndon words are STRICTLY smaller than their rotations, so "aa" is not
//     Lyndon; "a" and "aab" are. A single character always is.
//   The factorisation is non-increasing (`>=`), not strictly decreasing --
//     "aaa" factors as "a","a","a".
//   For the MAXIMUM rotation, flip every comparison. Do not reverse the
//     string; that answers a different question.
//   Minimal rotation with duplicates: several starts give the same rotation.
//     Any of them is a correct answer unless the problem asks for the
//     smallest index -- then keep the first.
//   s+s materialises 2n characters. For n = 1e7 index modulo n instead.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   lexicographically smallest rotation             min_rotation()
//   largest rotation                                same, comparisons flipped
//   canonical form of a NECKLACE / cyclic string    min_rotation(), then hash
//                                                   or compare -- two cyclic
//                                                   strings are equal iff
//                                                   their minimal rotations
//                                                   are
//   are s and t rotations of each other             same length AND equal
//                                                   minimal rotations (or: t
//                                                   is a substring of s+s)
//   lexicographically smallest SUFFIX                the last Lyndon factor
//   split into fewest non-increasing pieces          the factorisation IS that
//   number of distinct rotations of s                n / p, p = smallest
//                                                   period, when p divides n
//   de Bruijn sequence of order k                   concatenate all Lyndon
//                                                   words whose length divides
//                                                   k, in lex order
//   generate all Lyndon words up to length n        Duval's next-word
//                                                   successor, below
//   the Lyndon ARRAY (longest Lyndon word at i)     needed for the runs
//                                                   theorem; O(n) from SA
//
// NOT THIS:
//   smallest rotation but you also need SA anyway  -> suffix array of s+s is
//     fine and you have it already; Duval is just cheaper standalone.
//   "smallest string by rearranging"               -> sort the characters.
//   "smallest concatenation order"                 -> sort with the comparator
//     a+b < b+a. A completely different problem.
// ============================================================================

// ---- Chen-Fox-Lyndon factorisation. Returns the START INDEX of each factor,
//      plus n as a terminator, so factor t is s[out[t] .. out[t+1]-1].
vector<int> duval(const string &s) {
    int n = s.size(), i = 0;
    vector<int> out;
    while (i < n) {
        int j = i + 1, k = i;
        while (j < n && s[k] <= s[j]) {
            if (s[k] < s[j]) k = i;              // strictly bigger: restart
            else k++;                            // equal: period continues
            j++;
        }
        // now j - k is the period; emit as many whole blocks as fit
        while (i <= k) out.push_back(i), i += j - k;
    }
    out.push_back(n);
    return out;
}

// ---- lexicographically smallest suffix = the last Lyndon factor -----------
int smallest_suffix(const string &s) {
    vector<int> f = duval(s);
    return f[f.size() - 2];                      // last real factor start
}

// ---- minimal rotation: Duval over s+s, stop once the factor start passes n
int min_rotation(const string &s) {
    int n = s.size(), i = 0, best = 0;
    while (i < n) {
        best = i;
        int j = i + 1, k = i;
        while (j < 2 * n && s[k % n] <= s[j % n]) {
            if (s[k % n] < s[j % n]) k = i;
            else k++;
            j++;
        }
        while (i <= k) i += j - k;               // skip whole periods
    }
    return best;                                 // rotation starting here
}

// ---- maximal rotation: every comparison flipped ---------------------------
int max_rotation(const string &s) {
    int n = s.size(), i = 0, best = 0;
    while (i < n) {
        best = i;
        int j = i + 1, k = i;
        while (j < 2 * n && s[k % n] >= s[j % n]) {
            if (s[k % n] > s[j % n]) k = i;
            else k++;
            j++;
        }
        while (i <= k) i += j - k;
    }
    return best;
}

////////////////////////////////////////////////////////////////////////////////
// GENERATING ALL LYNDON WORDS of length <= n over an alphabet of size A, in
// lexicographic order, O(1) amortised per word (Duval's successor rule).
//
// Concatenating those whose LENGTH DIVIDES k gives the de Bruijn sequence of
// order k -- the shortest cyclic string containing every length-k word once.
////////////////////////////////////////////////////////////////////////////////
vector<string> lyndon_words(int n, int A) {
    vector<string> res;
    string w;
    w.push_back(0);                              // start with the letter 0
    while (!w.empty()) {
        res.push_back(w);
        // repeat w until length n, then increment the last non-max letter
        int old = w.size();
        while ((int)w.size() < n) w.push_back(w[w.size() - old]);
        while (!w.empty() && w.back() == A - 1) w.pop_back();
        if (!w.empty()) w.back()++;
    }
    return res;
}

string de_bruijn(int k, int A) {
    string res;
    for (string &w : lyndon_words(k, A))
        if (k % (int)w.size() == 0)
            for (char c : w) res.push_back('0' + c);
    return res;                                  // cyclic, length A^k
}
