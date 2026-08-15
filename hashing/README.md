# hashing — CP set (`long long` edition)

Same five standalone files, same algorithms. Now on `#define int long long` +
`int32_t main()`, with `unsigned long long` surviving only where it is
load-bearing — and flagged in each file's **TYPES** block, with the reason.

Every file also opens with a **MINIMAL BUILDS** block: *"for X, copy Y, Z, A"*,
with rough line counts, so you can write a cut-down version by hand instead of
pasting all of it.

## Files — what each one is for

| file | one line | reach for it when |
|---|---|---|
| `hash.cpp` | static sequence, O(1) range hash | substring equality, pattern search, suffix compare, borders/periods, distinct substrings, palindromes |
| `hash_seg.cpp` | the same, but the array changes | point or range updates + any of the above; **palindrome under updates**, which Manacher cannot do |
| `hash_tree.cpp` | trees × hashing | path-as-string, subtree shape, tree isomorphism, subtree/path label multisets |
| `hash_ms.cpp` | Zobrist, order-free | "same multiset?", "is it a permutation of 1..len?", anagram windows, "same up to relabelling" |
| `hash_2d.cpp` | grids | 2D pattern search, distinct submatrices, largest common square, mirror/rotation |

## Where `unsigned long long` survives, and why

| file | `ull` used for | reason |
|---|---|---|
| `hash.cpp` | `SplitMix` only | splitmix64 needs wraparound at 2^64 and a **logical** `>>`; signed makes the multiplies UB and sign-extends the shift |
| `hash_2d.cpp` | `SplitMix` only | same |
| `hash_seg.cpp` | **nowhere** | no `SplitMix` in this file; `ull` is not even `#define`d |
| `hash_ms.cpp` | every weight and accumulator | splitmix, *plus* the multiset hash is a sum that deliberately wraps — that wraparound is the group Z/2^64 the argument relies on, and signed overflow is UB |
| `hash_tree.cpp` | the Zobrist half only | `int` for `PathHash`/`SubtreeIso` (modular), `ull` for `SubtreeLabels`/`SubtreeLabelsDyn`. Do not unify them |

`__int128` is required in every `mul()`: two 61-bit factors make a 122-bit
product. Signed `__int128` is fine because both factors are non-negative.

## Why `long long` is *enough* for hash values

Not luck. A hash lives in `[0, MOD-1] = [0, 2^61-2]`, so `add()`'s intermediate
`a + b < 2^62 < 2^63-1`. Two bits of headroom, which is exactly what the
conditional subtract needs. Same for `mul()`'s `lo + hi`. Never widen this, and
never narrow `MOD`.

## Memory, now that everything is 8 bytes

| file | cost | note |
|---|---|---|
| `hash.cpp` | ~16 MB at `MAXN = 1e6+5` | unchanged — `ull` and `ll` are both 8 bytes |
| `hash_seg.cpp` | ~24 MB at `MAXN = 1e6+5` | three tables (`pw`, `ipw`, `geo`) |
| `hash_2d.cpp` | ~32 MB per 2000×2000 grid | `largest_common_square` holds two |
| `hash_tree.cpp` | `up[][]` is ~29 MB at n = 2e5, up from ~14 MB | **the one real regression** |
| `hash_ms.cpp` | `cnt[]` is 8 bytes per compressed value | |

Two places are safe to narrow if you hit a limit, and they are the only two:

- `hash_tree.cpp`: `up`, `par`, `dep`, `tin`, `tout`, `sz`, `order` hold node
  ids and depths, never hashes → `vector<int32_t>` is always enough.
- `hash_ms.cpp`: `MoState::cnt` holds counts ≤ n → `vector<int32_t>`.

Everything else is a hash. Leave it.

## Two `#define int long long` gotchas you will hit

1. **`min(n, 5)` stops compiling.** `n` is `long long`, `5` is `int`, template
   deduction fails. Write `min<int>(n, 5)` or `min(n, 5LL)`. Same for `max(1, x)`.
   This bit the test suite during validation; it will bite you mid-contest.
2. **Constructor overloads collapse.** `Hash(vector<ll>)` and `Hash(vector<int>)`
   became the same signature, so there is now only ONE vector constructor per
   struct. `vector<int>` *is* `vector<long long>`; convert a `vector<int32_t>`
   before passing it.

Also: `printf("%d", x)` on a 64-bit `int` is UB. Use `cout`, or cast.

## Validation

Every file was re-run against the original brute-force test suite with only
call sites and type declarations adapted: `t/l_core l_seg l_tree l_ms l_2d`.
All five pass under `-std=c++17` and `-std=c++20`, five runs each with fresh
random bases. Tests check against `std::string`, `set<string>`, exact canonical
tree forms, and direct O(n²) recomputation. Behaviour is identical to the
`ull` version.

## Traps, unchanged

- Base is random per run. Fixed bases are what anti-hash tests target.
- `enc()` never returns 0, else `"" == "\0" == "\0\0"`.
- `init_hash(MAXN - 1)`, never `init_hash(MAXN)` — the loop writes `pw[n]`.
- `max_overlap` is not monotone in k, so binary search on it is wrong.
  Palindromic radius *is* monotone, so bisecting there is fine.
- Manacher and Booth in `hash.cpp` are self-contained: for "longest
  palindromic substring" or "least rotation" you need none of the modular
  arithmetic in that file.
- `SECRET_ADD` / `SECRET_MUL` are global on purpose; two `SubtreeIso` objects
  with different secrets never compare equal.
- `SubtreeIso` is `SECRET_MUL * product + enc(label)`. The other placement is a
  *deterministic* collision.
- Zobrist SUM = multisets, XOR = sets / odd-parity.
- `Zobrist::can_be_pal` needs `prepare_canpal(a)` called once first.
- `HashSeg`'s lazy tag uses `-1` as "no tag", so assigned values must be `>= 0`.
- Doubling a string for rotations needs `MAXN >= 2n`.
