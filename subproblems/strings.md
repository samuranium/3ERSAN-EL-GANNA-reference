# STRINGS — subproblems

## PICK THE TOOL

| the question is about | tool | file |
|---|---|---|
| ONE pattern in one text | KMP or Z | `strings/Z kmp.cpp` |
| MANY patterns, one text | Aho–Corasick | `strings/aho_corasick.cpp` |
| substrings of ONE fixed text | suffix automaton | `strings/suffix_automaton.cpp` |
| comparing / sorting suffixes | suffix array + LCP | `strings/cleansuf.cpp` |
| "are these two ranges equal" | hashing | `hashing/hash.cpp` |
| palindromes | Manacher | `strings/separate manacher.cpp` |
| the text CHANGES between queries | hashing on a BIT/segtree | `hashing/hash_seg.cpp` |
| order does not matter (anagrams) | Zobrist multiset hash | `hashing/hash_ms.cpp` |
| a 2D grid of characters | 2D hashing | `hashing/hash_2d.cpp` |
| paths / subtrees of a tree | tree hashing | `hashing/hash_tree.cpp` |
| prefix set, many words | trie + DFS | `strings/trie.cpp` |

## BY QUESTION

| you see | do this | file |
|---|---|---|
| does p occur in s | KMP, or hash compare | `Z kmp.cpp` |
| all occurrences of p | KMP; or bitset matching | `DS/bitset.cpp` [4] |
| count occurrences of p in a range | bitset matching | `DS/bitset.cpp` |
| occurrences of **each** of k patterns | Aho, link-tree counting | `aho_corasick.cpp` |
| build strings avoiding a dictionary | DP over Aho automaton states | `aho_corasick.cpp` |
| shortest string containing all patterns | dp[node][mask] over the automaton | `aho_corasick.cpp` |
| number of DISTINCT substrings | suffix automaton, or n(n+1)/2 − Σlcp | `suffix_automaton.cpp` |
| k-th smallest distinct substring | SA + prefix sums of new substrings | `cleansuf.cpp` |
| longest repeated substring | max lcp, or max len with cnt ≥ 2 | `cleansuf.cpp` |
| longest substring occurring ≥ k times | window min over lcp | `suffix array.cpp` |
| longest common substring of 2 strings | feed t through s's automaton | `suffix_automaton.cpp` |
| longest common substring of k strings | concat with separators, SA window | `subsuf.cpp` |
| LCP of two suffixes | SA + sparse table | `cleansuf.cpp` |
| compare two arbitrary substrings | SA + LCP, O(1) | `cleansuf.cpp` |
| borders / periods of a string | prefix function | `Z kmp.cpp` |
| smallest period | n − pi[n−1] if it divides n | `Z kmp.cpp` |
| longest palindromic substring | Manacher | `separate manacher.cpp` |
| is s[l..r] a palindrome | hash forward vs reverse | `hashing/hash.cpp` |
| palindromes **with updates** | hashing on a segment tree | `hashing/hash_seg.cpp` |
| count palindromic substrings | Manacher, sum the radii | `separate manacher.cpp` |
| least / greatest cyclic rotation | Duval on s+s, O(n) O(1) | `strings/lyndon.cpp` |
| is t a rotation of s | t is a substring of s+s | — |
| are two ranges anagrams | Zobrist multiset hash | `hashing/hash_ms.cpp` |
| is a range a permutation of 1..len | Zobrist | `hashing/hash_ms.cpp` |
| can a range be rearranged to a palindrome | parity of each count | `hashing/hash_ms.cpp` |
| max xor / prefix questions on bit strings | binary trie | `DS/Trie/binary_trie.cpp` |
| 2D pattern in a grid | 2D hash | `hashing/hash_2d.cpp` |
| distinct submatrices | 2D hash | `hashing/hash_2d.cpp` |
| all stored words in lexicographic order | DFS on the trie, term before children | `strings/trie.cpp` |
| k-th smallest word / rank of a word | descend on subtree counts | `strings/trie.cpp` |
| autocomplete: words under a prefix | walk to the prefix, then DFS | `strings/trie.cpp` |
| longest stored word that prefixes q | one walk down | `strings/trie.cpp` |
| sort strings without comparisons | insert all, then collect() | `strings/trie.cpp` |

## WHICH ONE WHEN THEY OVERLAP

- **Hash vs suffix array** for "are these equal": hashing, every time. Shorter,
  faster, and the only failure mode is an adversarial test, which random bases
  handle.
- **Suffix automaton vs suffix array**: automaton for counting substrings of one
  string; array for comparing/ordering suffixes and for LCP.
- **Aho vs suffix automaton**: many patterns against one text → Aho. Many
  queries about one text's own substrings → automaton.
- **KMP vs Z**: interchangeable. Z is easier to reason about, KMP gives the
  border/period structure directly.

## TRAPS

- Fixed hash bases are what anti-hash tests target. Randomise per run —
  `hashing/hash.cpp` does.
- `enc()` must never return 0, or `""`, `"\0"` and `"\0\0"` collide.
- Manacher and Booth need none of the modular machinery around them; they are
  self-contained.
- Aho: `link` builds the automaton, `outlink` reports matches. Walking `link`
  per character is quadratic.
- Suffix automaton: reserve **2n** states; cloning is why.


---

# RESEARCH ADDITIONS

## STRUCTURE CHOICE — the decision above the algorithm

| you see | reach for | the tell |
|---|---|---|
| the string is revealed **incrementally**, answer after each char | suffix automaton or eertree | both are genuinely online, amortised O(1) per char. SA and Manacher are not |
| you need an explicit **tree over suffixes** (subtree = occurrence set, LCA = LCP) | suffix tree, built as the **link tree of the SAM of reversed s** | avoids Ukkonen entirely |
| you have SA + LCP and want suffix-tree reasoning | **Cartesian tree on the LCP array** — `DS/cartesian_tree.cpp` | internal nodes = LCP intervals = suffix tree internal nodes; a min-stack sweep is often enough |
| only equality / LCP of substrings, nothing lexicographic | hashing + binary search for LCP, O(log n) per query | the hard structure evaporates |
| **deterministic** LCP required (hacking risk) | SA + Kasai + sparse table | use when hashing is hackable |
| **many strings**, substring queries across the set | generalised SAM — NOT IN THE REPO; use SA of the concatenation with **distinct** separators (`strings/subsuf.cpp`) | a plain `last = root` reset is WRONG when the transition already exists |
| n ~ 1e6, memory tight | SA over SAM | SAM costs 2n states x sigma transitions; SA is four int arrays |
| alphabet huge / values to 1e9 | compress to ranks first | SA unchanged; SAM must switch to map transitions |
| SA of only k chosen suffixes of a huge text | sparse suffix sorting: hash + binary search LCP, O(k log^2 n) | do not materialise the full SA |

## PALINDROMES BEYOND MANACHER

| you see | reach for | the tell |
|---|---|---|
| count **distinct** palindromic substrings, or per-palindrome occurrences | **eertree** — `strings/eertree.cpp` | Manacher counts ALL palindromic substrings, not distinct. There are <= n distinct — that bound is why eertree is linear |
| palindromes **ending at each position** | eertree suffix-link chain (`num[last]` counts them), or Manacher + difference array | `strings/eertree.cpp` |
| min / count of **palindromic factorisations** | eertree with **series links**, O(n log n) — `strings/eertree.cpp` `PalFactor` | plain chain walk is O(n) per position -> O(n^2). Series links group the chain into O(log n) arithmetic progressions |
| palindromes common to two strings | joint eertree (build over s, continue over t sharing nodes) | same trick as generalised SAM |
| min chars to **prepend** to make a palindrome | longest palindromic PREFIX = KMP on `rev(s) + '#' + s` | mirror it for appending |
| min insertions/deletions to make a palindrome | `n - LCS(s, rev s)` | not a Manacher problem at all |
| longest palindromic **subsequence** | `LCS(s, rev s)` / interval DP | subsequence != substring — the most common misroute |
| count distinct palindromic **subsequences** | interval DP with the subtract-the-inner-block recurrence | O(n^2); no linear structure exists |
| palindrome with <= k mismatches | two-pointer with **LCE jumps** — `hashing/hash.cpp` + binary search, or `strings/cleansuf.cpp` (SA+RMQ) | each mismatch costs one LCE query -> O(k) per centre |

## PERIODICITY, BORDERS, LYNDON

| you see | reach for | the tell |
|---|---|---|
| ALL borders of s | the pi-chain from `pi[n-1]` | the chain can be Theta(n) long but decomposes into **O(log n) arithmetic progressions** |
| "s is a concatenation of copies of t" | `k = n - pi[n-1]`; valid only if **k divides n** | if k does not divide n, s is NOT a repetition, though k is still the smallest period |
| all periods of a **substring**, many queries | period queries: O(n) structure, O(log n) per query, periods as disjoint APs | Fine-Wilf forces the AP structure |
| two periods p, q with `len >= p + q - gcd(p,q)` | **Fine and Wilf**: gcd(p,q) is also a period | use it to prove only O(log n) candidate periods survive |
| minimal rotation, tight memory | **Duval on s+s** — O(n) time, O(1) extra space — `strings/lyndon.cpp` | beats SA of s+s by a large constant |
| maximum rotation | Duval, comparisons flipped — `strings/lyndon.cpp` `max_rotation` | same code, flipped comparator |
| count **distinct** rotations | `n / p` where p is the smallest period dividing n, else n | pure number theory once you have the period |
| split into the fewest lexicographically non-increasing pieces | **Lyndon factorisation (Duval)** — `strings/lyndon.cpp` | the definition is the answer |
| lexicographically smallest **suffix** | the last block of the Lyndon factorisation — `strings/lyndon.cpp` | O(n), no SA |
| find all squares / tandem repeats | **Main-Lorentz** — NOT IN THE REPO. n <= ~5000? for every period p, check each block boundary with an LCE forward and backward | the explicit list can be Theta(n^2) |
| find all **runs** (maximal repetitions) | runs theorem via the **Lyndon array** — NOT IN THE REPO; `strings/lyndon.cpp` has Duval, which is the factorisation the array comes from | the number of runs is < n; use it as an output-size sanity check |
| "every repetition in s" with n >= 1e5 | runs, not squares | squares are Theta(n^2); every square sits inside a run |

## PREFIX FUNCTION / Z BEYOND MATCHING

| you see | reach for | the tell |
|---|---|---|
| count occurrences of **every prefix** of s inside s | histogram of pi values, propagated **backwards** along the chain | `cnt[pi[i]-1] += cnt[i]` sweeping i downward |
| the text is astronomically long but **recursively defined** (Fibonacci word, Thue-Morse, `S -> S + f(S)`) | KMP automaton on the pattern, then DP over (recursion level, entry state) storing exit state + match count | never materialise the text; doubling gives O(log n) levels |
| count strings of length n avoiding **one** pattern | KMP automaton states -> linear recurrence -> **matrix power** | n up to 1e18 becomes a matrix power |
| avoiding a **set** of patterns | Aho-Corasick states -> matrix power over trie nodes | kill states whose fail chain hits a terminal |
| "longest prefix of s starting at position i" | **Z**, not pi | Z answers "match from here"; pi answers "border" |

## APPROXIMATE / EXOTIC MATCHING

| you see | reach for | the tell |
|---|---|---|
| pattern has **wildcards** | FFT: match at j iff `sum p_i * t_{i+j} * (p_i - t_{i+j})^2 == 0`, wildcard = 0 | three convolutions; the cubic form makes each term non-negative |
| **Hamming distance at every alignment** | one FFT per alphabet symbol (`math/fft_ntt.cpp`), or a bitset if sigma is small (`DS/bitset.cpp`) | cost sigma * n log n |
| k-mismatch matching, small k | "kangaroo jumping": k+1 LCE queries per alignment | O(nk) total, no FFT |
| exact match with m <= 64 | **Shift-And / Bitap** — NOT IN THE REPO, and it is 5 lines: `mask[c]` = bits where c occurs in p; per text char `D = ((D << 1) | 1) & mask[c]`, match when bit m-1 is set | one word op per text char |
| edit distance <= k with m <= 64 | bitap with k masks, O(nk/w) | — |
| edit distance where the answer is known <= k | **banded DP** over diagonals with abs(i-j) <= k, O(nk) — NOT IN THE REPO; it is the ordinary edit-distance DP with the inner loop clamped to `[i-k, i+k]` | Ukkonen; with an LCE oracle it drops to O(n + k^2) |
| "match ignoring which letters, only the pattern of repeats" (abab ~ cdcd) | **parameterized matching** — NOT IN THE REPO. Encode each char by the distance to its previous occurrence (0 if first), then run ordinary KMP on that array | — |
| "same relative order of values" | order-preserving matching: encode by rank-in-prefix, then KMP | "find the segment with the same shape" |
| regex / `*` and `?` matching, both long | NFA-over-positions DP, one **bitset row per pattern position** | O(nm/64) |

## SUBSEQUENCES

| you see | reach for | the tell |
|---|---|---|
| "is t a subsequence of s", **many** queries | **subsequence automaton** `nxt[i][c]` — `strings/subsequence_automaton.cpp` | O(len t) per query, O(n sigma) memory |
| shortest string **not** a subsequence of s | DP on the subsequence automaton — `strings/subsequence_automaton.cpp` | the answer length is O(log_sigma n) |
| count **distinct** subsequences | DP with last-occurrence subtraction — `strings/subsequence_automaton.cpp` | the subtraction IS the automaton's determinism |
| count subsequences satisfying a **regular** constraint | product DP: subsequence automaton x DFA of the constraint | works for "divisible by k", "avoids substring", etc. |
| LCS of two **permutations** / all-distinct strings | relabel by position in the first: **LCS becomes LIS** — `DP/lis.cpp` | — |
| LCS with n*m ~ 1e8 | bit-parallel LCS, O(nm/64) | length only |
| LCS where the number of matching pairs is small | Hunt-Szymanski, O((r+n) log n) | tiny alphabet overlap |
| "min ops = insert/delete only" | `n + m - 2*LCS` | not edit distance |

## TRIES AND BIT-TRIES

| you see | reach for |
|---|---|
| max/min XOR against a **prefix or range** | persistent binary trie — `DS/Trie/persistent_binary_trie.cpp` |
| count pairs with XOR < K, or k-th largest XOR | binary trie descent carrying subtree counts — `DS/Trie/persistent_binary_trie.cpp` |
| XOR-MST | Boruvka on a binary trie — `graph/MST/boruvka.cpp` `xor_mst_all` |
| dictionary that **changes** (patterns inserted online) | logarithmic rebuilding: O(log n) Aho automata of sizes 2^k, merge on insert |
| prefix-count queries over a **static** set | sort + binary search — no trie needed unless you also traverse |

## AHO-CORASICK EXTRAS

| you see | reach for | the tell |
|---|---|---|
| "how many times does pattern i occur", many patterns, offline | **Euler tour of the fail tree + BIT** — `strings/aho_fail_tree.cpp`; the reverse-BFS version in `strings/aho_corasick.cpp` is cheaper when you only need the totals | turns an outlink walk (which can be Theta(n sqrt m)) into O(1) per char |
| "how many times does pattern i occur inside pattern j" | all patterns in one trie, fail-tree Euler + BIT — `strings/aho_fail_tree.cpp` | never build one automaton per pair |
| lexicographically smallest string of length L containing **exactly** k patterns | DP over (Aho state, count), edges in alphabet order | the count dimension is what stops it being a shortest path |
| path in a grid/graph **spelling** a dictionary word | BFS/DFS over (cell, Aho state) | product graph is the generic move |

## SA / SAM EXTRAS

| you see | reach for |
|---|---|
| sum of LCP over all pairs of suffixes | monotonic stack on the LCP array — "sum of subarray minima" in disguise |
| distinct substrings of **every prefix** | SAM built incrementally, running sum of `len(v) - len(link(v))` |
| substrings occurring in **at least k of m** strings | generalised SA + sliding window over LCP counting distinct sources; or SAM link tree + segtree merge |
| occurrence count of **every** substring | SAM link tree subtree sums, propagated in decreasing len |
| set operations on occurrence positions | segment tree **merge** over the SAM link tree |
| "sort all rotations, read the last column" | BWT = SA of s + sentinel, take `s[sa[i]-1]` |
| shortest superstring, n <= 20 patterns | overlap graph + bitmask DP = Hamiltonian path |
| sequence containing every length-k string exactly once | de Bruijn: Eulerian circuit (`graph/SCC/eulerian_path.cpp`), or concatenate all Lyndon words of length dividing k (`strings/lyndon.cpp` `de_bruijn`) |

## TRANSFORMATIONS

| shape | really is | the move |
|---|---|---|
| "substring where every letter appears an **even** number of times" | prefix XOR of a sigma-bit parity mask, count equal prefix values | "at most one odd" = 27 lookups per index |
| "substring rearrangeable to a palindrome" | popcount(mask) <= 1 on the same prefix masks | same machinery |
| **period** of s | **border** of s: p is a period iff there is a border of length n-p | every period question is a pi-chain question |
| Aho-Corasick automaton | a DFA / graph | "count or construct strings that do or do not contain patterns" becomes walk counting: DP, BFS, Dijkstra, or matrix power for huge length |
| suffix automaton | a DAG whose source-paths are exactly the distinct substrings | counting / enumerating / k-th-ing substrings = path counting in a DAG |
| SAM suffix links | a tree with endpos sets as subtrees | occurrences become subtree aggregation |
| SA + LCP | a suffix tree (Cartesian tree on LCP) | "deepest node with >= k leaves" = a min-stack sweep |
| matching with wildcards / mismatch counts | polynomial multiplication | match score is a convolution |
| polynomial hashing | evaluating a polynomial at a random point over F_p | Schwartz-Zippel gives the bound, and explains why the **base** must be random |
| LCS of two permutations | LIS | relabel by position |
| edit distance | shortest path in a grid DAG | uniform weights -> banded DP; non-uniform -> Dijkstra |
| "min adjacent swaps to sort a string" | inversion count (BIT) | with duplicates, greedily match equal letters left to right |
| distinct chars, **any** two may swap | permutation cycles: `n - #cycles` | a different problem from adjacent swaps |
| bracket sequence | +-1 prefix sums | balanced iff min prefix >= 0 and total 0 |
| binary string, "equal 0s and 1s" | map 0 to -1, prefix sums, count equal values | also handles "at most k more 1s" |
| run-length encoded input | a sequence of O(runs) intervals | two-pointer over runs, never over characters |
| sorted word list in an unknown alphabet | a DAG on sigma characters, topological sort | a word before its own prefix is the contradiction case |
| "transform s to t one char at a time through a dictionary" | BFS on an implicit graph; add wildcard nodes to cut edges from n^2 to n*L | — |
| paths in a **tree** spelling strings | root it, hash paths, or Aho over the tree with a state stack + small-to-large / centroid | — |
| ordering pieces to minimise the concatenation | sort with the comparator `a+b < b+a` | this is a valid total order; `a < b` is not |
| cyclic / necklace problems | canonicalise by the **minimal rotation**, then hash | two necklaces are equal iff their minimal rotations are |

## TRAPS

- **Modulus 1e9+7 for hashing** with ~1e5-1e6 comparisons is nowhere near enough (birthday bound). Use 2^61-1 with `__int128`, or double hashing.
- **Natural `unsigned long long` overflow (mod 2^64)** is broken by a Thue-Morse string of ~2^11 chars for **every odd base**. Never use it.
- **Two fixed mods "so it is safe"** is still hackable. Randomise the **base** at runtime; keep base > sigma.
- **Comparing substrings of different lengths by hash** is meaningless. Multiply the shorter hash **up** by `pow[diff]`.
- **Different base/mod/power tables** between two strings means they are silently never equal.
- **`s + '#' + t` where `#` occurs in s or t**: pi can exceed len(s). Guard `pi[i] <= len(s)` explicitly.
- **`n - pi[n-1]`** is always the smallest period; s is a repetition only if it **divides n**.
- **Walking all borders per position** is O(n^2) on `a^n`. Precompute counts, or use the AP decomposition.
- **Manacher radius to (l,r) conversion**: the padded string has length 2n+1 and every off-by-one lives here.
- **Eertree without the length -1 imaginary root** never creates single-character palindromes. Allocate n+2 nodes, both roots.
- **Eertree DP walking the suffix-link chain per position** is amortised only for CONSTRUCTION — use series links for the DP.
- **SAM clone initialised with cnt = 1** inflates occurrence counts. Only `cur` states get 1; clones get 0.
- **Propagating SAM cnt in index order** is wrong — it must be decreasing `len` (counting sort).
- **Generalised SAM by just resetting last = root** is wrong when the transition already exists; handle `len(q) == len(p)+1` and the clone case.
- **One shared separator** when concatenating k strings creates fake crossing substrings. Use distinct separators.
- **SA sentinel `'$'`** must be strictly smaller than every character actually present.
- **LCP array convention**: `lcp[i]` is LCP(sa[i], sa[i-1]) in some libraries and LCP(sa[i], sa[i+1]) in others. A mismatch shifts every RMQ by one, silently.
- **Kasai with `k` reset inside the loop** becomes quadratic — the bound needs k to drop by at most 1 per step.
- **Aho memory**: nodes x sigma ints. sum of pattern lengths 1e6 with sigma 26 is ~104 MB.
- **Aho with duplicate patterns**: one terminal index per node loses all but one. Store a list.
- **DP over Aho states must use the fallback-resolved goto**, not the raw trie children.
- **KMP reports overlapping matches.** Max non-overlapping needs a greedy sweep.
- **FFT wildcard matching in doubles** dies on precision (terms reach 1e14+). Split into 15-bit halves or use NTT.
- **Bit-parallel / Hunt-Szymanski LCS gives the length only.** Reconstruction needs Hirschberg.
- **XOR-based multiset hashing** only tracks parity: {a,a} hashes to {}. Use summation of random 64-bit values.
- **`cin >> s` truncates at spaces**, and a leftover newline makes the first `getline` empty.
