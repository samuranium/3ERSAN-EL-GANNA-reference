// ============================================================================
// 2-SAT -- implication graph + SCC, O(V + E)
// ----------------------------------------------------------------------------
// WHEN: every constraint is an OR of at most TWO literals. Then satisfiability
//   and one witness assignment are both linear. Three literals per clause is
//   NP-hard -- there is no "just extend it".
//
// LITERAL ENCODING: variable i owns two nodes.
//     T(i) = 2i    "x_i is true"        F(i) = 2i+1   "x_i is false"
//   The negation of any literal v is v^1. That is the whole reason for this
//   layout -- no lookup table, no sign juggling.
//
// THE GRAPH: clause (a v b) means "if a is false then b must hold", so it is
//   TWO edges: ~a -> b and ~b -> a. imply() below adds an edge together with
//   its contrapositive, so add_or() is a single call.
//
// SATISFIABLE iff no variable has x and ~x in the same SCC. If they share a
//   component then x -> ~x -> x, so x implies its own negation.
//
// ############################################################################
// #  THE ONE LINE THAT FLIPS
// #
// #      bool val(int x) { return comp[T(x)] < comp[F(x)]; }
// #
// #  x should be TRUE when its component comes LATER in topological order.
// #  TARJAN numbers components in REVERSE topological order -- the first SCC
// #  popped is a sink -- so "later in topo order" means a SMALLER id, hence <.
// #  KOSARAJU numbers them in forward topological order, so the same code needs
// #  > instead. Using the wrong direction still reports SATISFIABLE correctly
// #  and then prints an assignment that violates a clause.
// ############################################################################
//
// OTHER PITFALLS:
//   N must be >= 2 * (number of variables). Sizing it by variables is the
//     classic silent overflow here.
//   MULTI-TEST: call clear(n) first. Every array here is global.
//   tarjan() is recursive over 2n nodes. For big n use the iterative version
//     in graph/SCC/tarjan - bridges.cpp.
//   Variables are 0-indexed in this file.
//
// ----------------------------------------------------------------------------
// ENCODING TABLE -- everything reduces to add_or / imply
// ----------------------------------------------------------------------------
//   (a v b)              add_or(a, b)
//   force a true         force(a)                 == add_or(a, a)
//   a -> b               imply(a, b)              (adds ~b -> ~a for you)
//   a == b               imply(a,b); imply(b,a);
//   a != b  (xor)        imply(a, b^1); imply(b, a^1);
//   NOT (a AND b)        add_or(a^1, b^1)
//   at most one of a,b   add_or(a^1, b^1)         same thing
//
//   AT MOST ONE of k literals -- pairwise is O(k^2) clauses and will TLE past
//     a few thousand. Linear version: add prefix variables p_1..p_k with
//         p_i means "one of the first i is true"
//         imply(a_i, p_i);  imply(p_{i-1}, p_i);  imply(a_i, p_{i-1}^1)
//     That is O(k) clauses.
//
//   AT LEAST ONE of k literals -- NOT directly expressible. A k-way OR is not a
//     2-clause. Same prefix-variable trick, or rethink the model. This is the
//     most common place a 2-SAT reduction quietly fails.
//
// ----------------------------------------------------------------------------
// THE DISGUISE: 2-SAT shows up whenever every item has exactly TWO choices and
// constraints are pairwise. Each item -> one boolean.
//   place each object at position A or B, no two may overlap
//   pick one of two times per event, some pairs conflict
//   orient each edge one way or the other under pairwise rules
//   2-colour with "these two must differ / must match" constraints
// If an item has THREE or more choices, this is not 2-SAT -- look at flows or
// matching instead.
//
// NOT THIS: anything asking to MINIMISE or MAXIMISE. 2-SAT answers "is there
//   an assignment", not "the best assignment". Optimisation over 2-clauses is
//   min-cut / project selection territory -- see graph/flows/.
// ============================================================================
const int N = 200'123;              // >= 2 * number of variables
vector<int> g[N];
int comp[N], low[N], num[N], dfn, nc;
bool ins[N];
stack<int> stk;

int T(int x) { return 2 * x; }      // literal "x is true"
int F(int x) { return 2 * x + 1; }  // literal "x is false"   (neg of v is v^1)

void imply(int u, int v) { g[u].push_back(v), g[v ^ 1].push_back(u ^ 1); }
void add_or(int a, int b) { imply(a ^ 1, b); }   // (a v b)
void force(int a) { imply(a ^ 1, a); }           // a must hold

// MULTI-TEST: call before each case. Everything here is global, so without
// this the next case inherits the previous implication graph.
void clear(int n) {
    for (int i = 0; i < 2 * n; i++)
        g[i].clear(), comp[i] = low[i] = num[i] = 0, ins[i] = 0;
    dfn = nc = 0;
    while (stk.size()) stk.pop();
}

void tarjan(int u) {
    num[u] = low[u] = ++dfn;
    stk.push(u), ins[u] = 1;
    for (int v : g[u])
        if (!num[v]) tarjan(v), low[u] = min(low[u], low[v]);
        else if (ins[v]) low[u] = min(low[u], num[v]);
    if (num[u] == low[u]) {
        for (;;) {
            int x = stk.top(); stk.pop();
            ins[x] = 0, comp[x] = nc;
            if (x == u) break;
        }
        nc++;
    }
}
// n = number of VARIABLES. false => contradiction.
bool satisfiable(int n) {
    for (int i = 0; i < 2 * n; i++) if (!num[i]) tarjan(i);
    for (int i = 0; i < n; i++) if (comp[T(i)] == comp[F(i)]) return false;
    return true;
}
// value of variable x in the witness assignment. See the box above.
bool val(int x) { return comp[T(x)] < comp[F(x)]; }
