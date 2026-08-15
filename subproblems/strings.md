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
| prefix set, many words | trie | `DS/Trie/string_trie.cpp` |

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
| least / greatest cyclic rotation | Booth, or SA of s+s | `hashing/hash.cpp` |
| is t a rotation of s | t is a substring of s+s | — |
| are two ranges anagrams | Zobrist multiset hash | `hashing/hash_ms.cpp` |
| is a range a permutation of 1..len | Zobrist | `hashing/hash_ms.cpp` |
| can a range be rearranged to a palindrome | parity of each count | `hashing/hash_ms.cpp` |
| max xor / prefix questions on bit strings | binary trie | `DS/Trie/binary_trie.cpp` |
| 2D pattern in a grid | 2D hash | `hashing/hash_2d.cpp` |
| distinct submatrices | 2D hash | `hashing/hash_2d.cpp` |

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
