# GAME THEORY — subproblems

Code: `game/grundy.cpp`.

## FIRST: IS IT IMPARTIAL?

All four must hold, or Sprague–Grundy does not apply:

1. players alternate, both play optimally
2. **both have the same moves available** — not chess, not "I move white"
3. finite, no draws, no repeated positions
4. **normal play**: the player who cannot move loses

If the players have different move sets it is *partizan* and none of this works.

## THE THEORY

```
G(pos) = mex{ G(p) : p reachable in one move }
G == 0            <=>  the player TO MOVE loses
G(A + B + ...)    =    G(A) xor G(B) xor ...
```

`mex` = smallest non-negative integer not in the set; `mex{} = 0`.

**Trigger:** several independent piles/heaps/components, and a move touches
exactly one of them. If a move can touch two, the xor rule does not apply.

## KNOWN GAMES

| game | value |
|---|---|
| Nim, pile of n | `n` — whole game is xor of sizes |
| take 1..k from a pile | `n mod (k+1)` |
| take any s ∈ S | table by mex — `grundy_subtraction()` |
| **Misère** Nim | same as Nim **unless every pile is 1**; then the parity of the pile count decides |
| Staircase Nim | xor of the **odd-indexed** steps only |
| Wythoff (two piles) | losing positions are `(⌊kφ⌋, ⌊kφ²⌋)` |
| split a pile into unequal parts | mex over all splits |
| edge deletion on a graph, rooted | Colon + Fusion — `grundy_graph()` |
| coin turning | xor of the single-coin values over the heads |

## RECOGNITION

| the statement | think |
|---|---|
| "two players take turns, last to move wins" | impartial, normal play → Grundy |
| "the one who cannot move wins" | **misère** — different, check the all-ones case |
| several piles / rows / components | xor the Grundy values |
| one pile only, "who wins" | win/lose recursion is enough; skip mex |
| large n, no obvious pattern | compute G for n = 0..40 and **look for a period** — impartial games are periodic remarkably often |
| both players have different pieces | partizan — not this |
| a score is accumulated, not just win/lose | minimax DP, not Grundy |

## TRAPS

- Grundy values are **not** 0/1 and not the move count. Computing win/lose per
  component and xoring the booleans is wrong when components combine.
- Misère is not "flip the answer". Only the all-piles-of-1 case differs in Nim,
  and general misère theory is much harder.
- If the state space is large, find the period rather than the table.
- A move that affects two components breaks the sum rule entirely.

---

# RESEARCH ADDITIONS


## GAME THEORY

| you see | reach for | the tell |
|---|---|---|
| move = remove from a pile **or split** a pile | Lasker's Nim / take-and-break; Grundy via mex over both move classes | the Grundy sequence is almost always eventually periodic — find the period |
| remove from **up to k-1 piles** at once | **Moore's Nim_k**: losing iff at every bit the count of piles with that bit set is 0 mod k | k=2 degenerates to ordinary Nim |
| remove at most twice the previous move | **Fibonacci Nim**: losing iff n is a Fibonacci number; strategy via Zeckendorf | the move limit depends on the last move |
| take between 1 and k | Bash game: losing iff `n mod (k+1) == 0` | trivial but frequently disguised |
| poset / "eat a corner and everything above it" (Chomp) | **strategy stealing**: the first player wins for any board bigger than 1x1 | the answer is "First" with no constructible strategy — do not search for one |
| players have **different** move sets | Sprague-Grundy does NOT apply — partizan; DP the state | "Alice may only ..., Bob may only ..." |
| draws possible, or positions repeat | retrograde analysis: BFS backwards with out-degree counters | plain memo DP mislabels draws as losses |
| players accumulate **score** | plain minimax DP — Grundy is meaningless | Grundy only encodes who moves last |
| a 2D turning game / product of two coordinate games | **Tartan theorem**: Grundy = nim-PRODUCT of the coordinate Grundy values | nim-product, not XOR |
| huge N, tiny move set | brute Grundy for small N, detect the period, extrapolate | subtraction-game Grundy sequences are eventually periodic |
| a move the opponent can immediately undo | pairing / mirroring argument | symmetry beats Grundy computation |
