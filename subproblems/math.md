# MATH — subproblems

Number-theory specific questions (primes, divisors, factorisation) are in
`number_theory.md` and route to the other team's PDF.

## MODULAR

| you see | do this | file |
|---|---|---|
| `a^e mod p` | fast power | `math/modular.cpp` |
| `1/a mod p`, p prime | `fp(a, p−2)` | `math/modular.cpp` |
| `1/a mod m`, any m | extended gcd, reports failure | `math/extgcd.cpp` |
| every inverse 1..n | O(n) table | `math/modular.cpp` |
| mod > 2³¹ | `__int128` mulmod | `math/modular.cpp` |
| huge exponent, `gcd(a,m)=1` | reduce e mod φ(m) | `math/phi.cpp` |
| huge exponent, gcd ≠ 1 | e mod φ(m) **+ φ(m)** | `math/phi.cpp` |
| `a^x = b mod m`, find x | baby-step giant-step | `math/bsgs.cpp` |
| `ax + by = c` over integers | diophantine | `math/extgcd.cpp` |
| `x ≡ r1 (m1)`, `x ≡ r2 (m2)` | CRT, non-coprime safe | `math/crt.cpp` |
| answer mod a non-prime | factor it, CRT the parts | `math/crt.cpp` |

## LINEAR ALGEBRA

| you see | do this | file |
|---|---|---|
| solve a linear system mod p | Gauss | `math/gauss.cpp` |
| count solutions | p^(free variables) | `math/gauss.cpp` |
| rank of a matrix | pivots | `math/gauss.cpp` |
| determinant | `det_mod` | `math/gauss.cpp` |
| matrix inverse | `inverse_mod` | `math/gauss.cpp` |
| parity / xor constraints | GF(2) Gauss with bitsets | `math/gauss.cpp` |
| max xor of a subset | xor basis | `DP/sub-problems xorbasis.cpp` |
| "every X an even number of times" | parity vectors + xor basis | `DP/sub-problems xorbasis.cpp` |
| number of spanning trees | Kirchhoff = a determinant | `graph/kirchhoff.cpp` |
| linear recurrence, huge n | matrix power | `DP/matrix/matrix expo.cpp` |
| probabilities / expected values with cycles | Gauss over the reals | `math/gauss.cpp` |
| random walk hitting time | same | `math/gauss.cpp` |

## POLYNOMIALS / CONVOLUTION

| you see | do this | file |
|---|---|---|
| multiply polynomials | NTT (998244353) | `math/fft_ntt.cpp` |
| ... modulo 1e9+7 | 3 NTTs + CRT, or split FFT | `math/fft_ntt.cpp` |
| ... real or floating data | FFT | `math/fft_ntt.cpp` |
| count pairs by SUM | convolution | `math/fft_ntt.cpp` |
| count pairs by XOR / AND / OR | FWHT, **not** FFT | their Math PDF |
| big integer multiplication | FFT on digits, then carry | `math/fft_ntt.cpp` |
| string matching with wildcards | 3 convolutions | `math/fft_ntt.cpp` |
| polynomial inverse / exp / division | Newton on top of NTT | their Math PDF |

## BITS

All of it: `math/bitwise.cpp`, and `DS/bitset.cpp` for the ÷64 loop shapes.

| you see | do this |
|---|---|
| lowest set bit | `x & -x` |
| clear lowest set bit | `x & (x−1)` |
| index of the highest bit | `__lg(x)` |
| count set bits | `__builtin_popcountll` |
| iterate submasks of a mask | `for (s = m; ; s = (s−1) & m)` |
| iterate supermasks | `for (s = m; s < 1<<n; s = (s+1) \| m)` |
| all k-subsets | Gosper's hack |
| xor of 1..n | closed form by `n mod 4` |
| is nCr odd | `(n & r) == r` |
| a + b without `+` | `(a^b) + 2(a&b)` |
| gray code | `i ^ (i>>1)` |

## TRAPS

- `1 << 63` is UB. `1LL << 63`.
- `__builtin_ctz(0)` and `__builtin_clz(0)` are undefined.
- `a & 1 == 0` parses as `a & (1 == 0)`.
- There is **no NTT modulo 1e9+7**. 998244353 = 119·2²³ + 1.
- FFT precision: `n · max|a| · max|b|` must stay under ~9e15.
- `divi()` needs a **prime** modulus; use `inv_mod` otherwise.
- Catalan/binomial division is modular — multiply by the inverse, never `/`.
