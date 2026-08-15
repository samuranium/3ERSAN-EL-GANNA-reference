// ============================================================================
// GAUSSIAN ELIMINATION -- solve, rank, determinant, inverse.  O(n^3)
// ----------------------------------------------------------------------------
// Three variants, and picking the wrong one is the usual mistake:
//   MOD P   exact, needs a PRIME modulus for the inverse. Default choice.
//   REAL    doubles + partial pivoting + EPS. Only when the problem is
//           genuinely continuous (geometry, probability, physics).
//   GF(2)   rows are bitsets, elimination is one xor. O(n*m/64).
//           For a BASIS only (no right-hand side) use the xor basis instead:
//           DP/sub-problems xorbasis.cpp.
//
// ############################################################################
// #  FIND THE PIVOT BEFORE YOU INVERT IT
// #
// #  A zero pivot means the column is a FREE VARIABLE, not an error -- skip
// #  the column and keep the same row. Computing modInverse(0) first is the
// #  standard slip: it returns 0 rather than failing, so the bug survives to
// #  the answer instead of crashing.
// #
// #  REAL version: pivot on the LARGEST |value| in the column, not the first
// #  nonzero. Without partial pivoting a tiny pivot amplifies rounding until
// #  the answer is noise -- and it still looks like a plausible number.
// ############################################################################
//
// RETURN CONVENTION of solve_mod: -1 = no solution, otherwise the number of
//   FREE variables. 0 free means the solution is unique; k free means p^k
//   solutions exist and sol[] is one particular member (free vars set to 0).
//   Reporting "unique" when free > 0 is the classic wrong answer.
//
// PITFALLS:
//   mod must be PRIME for solve_mod / det_mod / inverse_mod -- they use
//     Fermat. Composite modulus: math/extgcd.cpp inv_mod, and pivots must be
//     chosen coprime with the modulus, which is a different algorithm.
//   The consistency check runs against the REDUCED rows, which is valid: row
//     operations preserve the solution set, so a zero row with nonzero RHS is
//     exactly "no solution".
//   n^3 at n = 500 is 1.25e8 with a modmul inside -- that is the practical
//     ceiling. GF(2) with bitsets pushes it to a few thousand.
//   The matrix is taken BY VALUE. These destroy it; that is intentional.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   solve a linear system mod p          solve_mod()
//   count solutions of a system          p^(free vars), or 0
//   rank of a matrix                     m - free, or count pivots
//   determinant                          det_mod()
//   matrix inverse                       inverse_mod()
//   number of spanning trees             Kirchhoff -- graph/kirchhoff.cpp,
//                                        which is det_mod of a minor
//   xor system / GF(2) equations         gauss_gf2(), or the xor basis
//   "assign 0/1 to satisfy parity        GF(2) -- every parity constraint is
//    constraints"                        one row
//   probability / expected value with    REAL version; states become
//     cycles in the state graph          variables, transitions become rows
//   random walk hitting times            same shape
//   fit a polynomial through points      REAL, or Lagrange interpolation
//                                        directly (cheaper, O(n^2))
//   linear recurrence from terms         Berlekamp-Massey is the right tool,
//                                        not this
// ============================================================================
// needs mod, add, subt, mult, fp from math/modular.cpp

// Solve a*x = b, given as an n x (m+1) augmented matrix.
// Returns -1 if inconsistent, else the number of free variables.
int solve_mod(vector<vector<int>> a, vector<int> &sol) {
    int n = a.size(), m = (int)a[0].size() - 1;
    vector<int> where(m, -1);
    for (int col = 0, row = 0; col < m && row < n; col++) {
        int sel = -1;
        for (int i = row; i < n; i++)
            if (a[i][col]) { sel = i; break; }
        if (sel == -1) continue;                      // free variable
        swap(a[sel], a[row]);
        where[col] = row;
        int inv = fp(a[row][col], mod - 2);           // pivot is nonzero HERE
        for (int j = col; j <= m; j++) a[row][j] = mult(a[row][j], inv);
        for (int i = 0; i < n; i++)
            if (i != row && a[i][col]) {
                int f = a[i][col];
                for (int j = col; j <= m; j++)
                    a[i][j] = subt(a[i][j], mult(f, a[row][j]));
            }
        row++;
    }
    sol.assign(m, 0);
    for (int j = 0; j < m; j++)
        if (where[j] != -1) sol[j] = a[where[j]][m];
    for (int i = 0; i < n; i++) {                     // consistency
        int s = 0;
        for (int j = 0; j < m; j++) s = add(s, mult(sol[j], a[i][j]));
        if (s != a[i][m]) return -1;
    }
    int freev = 0;
    for (int j = 0; j < m; j++) freev += (where[j] == -1);
    return freev;
}
// determinant of an n x n matrix mod p. 0 means singular.
int det_mod(vector<vector<int>> a) {
    int n = a.size(), res = 1;
    for (int i = 0; i < n; i++) {
        int sel = -1;
        for (int j = i; j < n; j++)
            if (a[j][i]) { sel = j; break; }
        if (sel == -1) return 0;                      // singular
        if (sel != i) swap(a[sel], a[i]), res = res ? mod - res : 0;
        res = mult(res, a[i][i]);
        int inv = fp(a[i][i], mod - 2);
        for (int j = i; j < n; j++) a[i][j] = mult(a[i][j], inv);
        for (int j = i + 1; j < n; j++)
            if (a[j][i]) {
                int f = a[j][i];
                for (int k = i; k < n; k++)
                    a[j][k] = subt(a[j][k], mult(f, a[i][k]));
            }
    }
    return res;
}
// inverse of an n x n matrix mod p; empty vector if singular
vector<vector<int>> inverse_mod(vector<vector<int>> a) {
    int n = a.size();
    vector<vector<int>> inv(n, vector<int>(n, 0));
    for (int i = 0; i < n; i++) inv[i][i] = 1;
    for (int i = 0; i < n; i++) {
        int sel = -1;
        for (int j = i; j < n; j++)
            if (a[j][i]) { sel = j; break; }
        if (sel == -1) return {};                     // singular
        swap(a[sel], a[i]), swap(inv[sel], inv[i]);
        int iv = fp(a[i][i], mod - 2);
        for (int j = 0; j < n; j++)
            a[i][j] = mult(a[i][j], iv), inv[i][j] = mult(inv[i][j], iv);
        for (int r = 0; r < n; r++)
            if (r != i && a[r][i]) {
                int f = a[r][i];
                for (int j = 0; j < n; j++) {
                    a[r][j] = subt(a[r][j], mult(f, a[i][j]));
                    inv[r][j] = subt(inv[r][j], mult(f, inv[i][j]));
                }
            }
    }
    return inv;
}

// ---------------------------------------------------------------------------
// GF(2): rows are bitsets of width m, column m holds the RHS. O(n*m/64).
// Returns -1 inconsistent, else the number of free variables; sol is one
// particular solution.
// ---------------------------------------------------------------------------
const int GB = 512;                                   // >= m + 1
int gauss_gf2(vector<bitset<GB>> a, int n, int m, bitset<GB> &sol) {
    vector<int> where(m, -1);
    for (int col = 0, row = 0; col < m && row < n; col++) {
        int sel = -1;
        for (int i = row; i < n; i++)
            if (a[i][col]) { sel = i; break; }
        if (sel == -1) continue;
        swap(a[sel], a[row]);
        where[col] = row;
        for (int i = 0; i < n; i++)
            if (i != row && a[i][col]) a[i] ^= a[row];
        row++;
    }
    sol.reset();
    for (int j = 0; j < m; j++)
        if (where[j] != -1) sol[j] = a[where[j]][m];
    for (int i = 0; i < n; i++) {
        int s = 0;
        for (int j = 0; j < m; j++) s ^= (sol[j] & a[i][j]);
        if (s != (int)a[i][m]) return -1;
    }
    int freev = 0;
    for (int j = 0; j < m; j++) freev += (where[j] == -1);
    return freev;
}

// ---------------------------------------------------------------------------
// REAL version -- partial pivoting, EPS. Only for genuinely continuous data.
// Same return convention. EPS of 1e-9 suits values around 1; scale it with
// your magnitudes.
// ---------------------------------------------------------------------------
const double EPS = 1e-9;
int solve_real(vector<vector<double>> a, vector<double> &sol) {
    int n = a.size(), m = (int)a[0].size() - 1;
    vector<int> where(m, -1);
    for (int col = 0, row = 0; col < m && row < n; col++) {
        int sel = row;
        for (int i = row; i < n; i++)                 // LARGEST pivot, not first
            if (fabs(a[i][col]) > fabs(a[sel][col])) sel = i;
        if (fabs(a[sel][col]) < EPS) continue;
        swap(a[sel], a[row]);
        where[col] = row;
        for (int i = 0; i < n; i++)
            if (i != row) {
                double f = a[i][col] / a[row][col];
                for (int j = col; j <= m; j++) a[i][j] -= a[row][j] * f;
            }
        row++;
    }
    sol.assign(m, 0);
    for (int j = 0; j < m; j++)
        if (where[j] != -1) sol[j] = a[where[j]][m] / a[where[j]][j];
    for (int i = 0; i < n; i++) {
        double s = 0;
        for (int j = 0; j < m; j++) s += sol[j] * a[i][j];
        if (fabs(s - a[i][m]) > EPS) return -1;
    }
    int freev = 0;
    for (int j = 0; j < m; j++) freev += (where[j] == -1);
    return freev;
}
