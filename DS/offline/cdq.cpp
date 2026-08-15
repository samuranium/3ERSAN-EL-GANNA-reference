// ============================================================================
// CDQ DIVIDE AND CONQUER -- 3D partial order / dynamic 2D counting, offline
// ----------------------------------------------------------------------------
// The problem shape: count pairs (or triples) satisfying THREE simultaneous
// inequalities. A BIT handles two dimensions; CDQ removes the third.
//
//     dimension 1   handled by SORTING the input once
//     dimension 2   handled by the DIVIDE AND CONQUER over indices
//     dimension 3   handled by a BIT inside each merge
//
// The recursion: solve(l, mid), solve(mid+1, r), then account for every
// CROSS pair -- left element influences right element. Inside the merge both
// halves are already sorted by dimension 2, so a two-pointer sweep feeds the
// left half into the BIT and queries from the right half.
//
// The key reframing: when the problem is "insert, then query, then insert...",
// dimension 1 is TIME. CDQ is then "let all earlier operations affect all
// later ones", which is why it replaces a dynamic 2D structure.
//
// ############################################################################
// #  ONLY CROSS CONTRIBUTIONS BELONG IN THE MERGE
// #
// #  solve(l,mid) and solve(mid+1,r) have already counted the pairs inside
// #  each half. The merge must add ONLY left->right pairs. Feeding the whole
// #  range into the BIT double counts everything, and the answer comes out
// #  roughly 2x -- which looks like an off-by-a-factor bug anywhere else.
// ############################################################################
//
// ############################################################################
// #  CLEAR THE BIT BY UNDOING, NEVER BY memset
// #
// #  After the merge, remove exactly the entries you inserted -- walk the
// #  same left elements again with -1. A memset of the whole BIT is O(maxval)
// #  per merge and turns O(n log^2 n) into O(n * maxval).
// #
// #  Equally: the two-pointer that feeds the BIT must run to the END of the
// #  left half before the last right query, or the tail of the left half is
// #  silently dropped.
// ############################################################################
//
// PITFALLS:
//   Duplicates in dimension 1: sort, then group equal elements and count them
//     together, or a triple equal to another is miscounted (<= vs <).
//   The comparison must be consistent: decide up front whether you count
//     strict (<) or non-strict (<=) in each dimension. Mixing them is the
//     usual source of an off-by-a-few answer.
//   Coordinate-compress dimension 3 before indexing the BIT.
//   CDQ is OFFLINE. If the queries are forced online, this is disqualified.
//   Recursion depth is log n -- safe. The BIT is shared across all levels.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   count triples dominated in 3 dimensions          solve() below
//   "for each element, how many are <= it in all 3"   the per-element counts
//   number of inversions                             2D -- a BIT alone is
//                                                    enough, do not use CDQ
//   dynamic 2D point counting (insert + query)       time is dimension 1
//   "add a point, then count points in a rectangle"   same
//   k-dimensional dominance                          nest CDQ, one level per
//                                                    extra dimension
//   LIS in 3 dimensions                              CDQ where the merge does
//                                                    a max-BIT instead of a
//                                                    counting one
//   offline range-update + range-query in 2D          time + CDQ + BIT
//   "how many earlier operations affect this query"   the generic framing
//
// NOT THIS:
//   only two dimensions      -> sort by one, BIT the other. Half the code.
//   forced online            -> a 2D structure (BIT of sorted vectors,
//                               k-d tree, persistent segtree).
//   the third dimension has few distinct values -> just loop over them.
// ============================================================================
const int MAXV = 200005;

// ---- BIT over dimension 3 -------------------------------------------------
int bit[MAXV];
void upd(int i, int v) { for (; i < MAXV; i += i & -i) bit[i] += v; }
int  qry(int i) { int s = 0; for (; i > 0; i -= i & -i) s += bit[i]; return s; }

struct P { int a, b, c, cnt, ans; };   // cnt = how many identical copies

// after sorting by a, then b, then c: count for each element how many others
// are <= it in ALL THREE dimensions (excluding itself)
vector<P> p;

bool by_b(const P &x, const P &y) {
    if (x.b != y.b) return x.b < y.b;
    return x.c < y.c;
}

void cdq(int l, int r) {
    if (l >= r) return;
    int mid = l + r >> 1;
    cdq(l, mid), cdq(mid + 1, r);
    // both halves are sorted by (b, c); sweep and count LEFT -> RIGHT only
    int i = l;
    for (int j = mid + 1; j <= r; j++) {
        while (i <= mid && p[i].b <= p[j].b) upd(p[i].c, p[i].cnt), i++;
        p[j].ans += qry(p[j].c);                     // cross pairs only
    }
    for (int k = l; k < i; k++) upd(p[k].c, -p[k].cnt);   // UNDO, not memset
    inplace_merge(p.begin() + l, p.begin() + mid + 1, p.begin() + r + 1, by_b);
}

// full pipeline: dedupe identical triples, sort by a, run CDQ.
// After this, p[i].ans + p[i].cnt - 1 is the count for each of its copies.
void solve_dominance() {
    sort(p.begin(), p.end(), [](const P &x, const P &y) {
        if (x.a != y.a) return x.a < y.a;
        if (x.b != y.b) return x.b < y.b;
        return x.c < y.c;
    });
    // group identical triples so that ties are not undercounted
    vector<P> q;
    for (auto &e : p) {
        if (!q.empty() && q.back().a == e.a && q.back().b == e.b && q.back().c == e.c)
            q.back().cnt += e.cnt;
        else q.push_back(e);
    }
    p.swap(q);
    for (auto &e : p) e.ans = 0;
    cdq(0, (int)p.size() - 1);
}
