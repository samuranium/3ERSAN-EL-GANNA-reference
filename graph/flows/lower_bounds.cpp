// ============================================================================
// FLOW WITH LOWER BOUNDS -- every edge must carry at least L
// ----------------------------------------------------------------------------
// Each edge has a range [lo, hi], not just a capacity. The reduction:
//
//   1. Send `lo` on every edge FOR FREE. That satisfies the lower bounds but
//      leaves each vertex unbalanced: excess[v] = (lo flowing in) - (lo out).
//   2. Give the edge residual capacity hi - lo.
//   3. Add a SUPER source SS and SUPER sink TT. For each v:
//          excess[v] > 0  ->  edge SS -> v  with capacity  excess[v]
//          excess[v] < 0  ->  edge v -> TT  with capacity -excess[v]
//   4. Max-flow SS -> TT. A feasible circulation exists IFF that flow
//      SATURATES every SS edge (equivalently, equals the total positive
//      excess).
//
// For a feasible s-t FLOW rather than a circulation, first add an edge
// t -> s with capacity INF, run the above, then the value of the s-t flow is
// the flow on that t->s edge.
//
// ############################################################################
// #  FEASIBILITY IS "ALL LOWER-BOUND EDGES SATURATED", NOT "FLOW > 0"
// #
// #  The answer to "is there a valid flow" is
// #      maxflow(SS, TT) == sum of all POSITIVE excesses
// #  A partial flow means some lower bound cannot be met. Checking only that
// #  the flow is non-zero, or forgetting the comparison entirely, reports
// #  every infeasible instance as feasible.
// ############################################################################
//
// ############################################################################
// #  THE t -> s EDGE MUST BE ADDED BEFORE THE FEASIBILITY RUN, AND REMOVED
// #  BEFORE THE MAXIMISATION RUN
// #
// #  MIN feasible s-t flow: after the feasibility run, DELETE the t->s edge
// #  (set its capacity to 0) and push flow back from t to s -- the answer is
// #  the original t->s flow MINUS what you pushed back.
// #  MAX feasible s-t flow: keep everything, then run maxflow(s, t) again on
// #  the residual graph and ADD it to the t->s flow.
// #  Doing the second run without the first gives a flow that violates the
// #  lower bounds.
// ############################################################################
//
// PITFALLS:
//   The final flow on an edge is lo + (flow found in the reduced network).
//     Reporting the reduced flow alone silently drops every lower bound.
//   excess[] must be accumulated over ALL edges before adding the SS/TT edges.
//   lo > hi on any edge is instantly infeasible; check it.
//   Self-loops with lo > 0 are infeasible unless you special-case them.
//   INF for the t->s edge must not overflow when summed: 1e18 is fine as a
//     single capacity, but do not add several of them.
//
// ----------------------------------------------------------------------------
// SUBPROBLEMS
// ----------------------------------------------------------------------------
//   does a valid circulation exist                 feasible_circulation()
//   each edge must carry at least L                 this
//   "every task must be done at least k times"      lower bound k on that edge
//   "each person works between a and b hours"        lower bound a, capacity b
//   min feasible s-t flow                           feasibility run, then push
//                                                   back from t to s
//   max feasible s-t flow                           feasibility run, then a
//                                                   normal maxflow s->t
//   no source or sink, everything balances          circulation with demands:
//                                                   the same construction with
//                                                   no s/t at all
//   vertex has a demand (must consume d units)      model as a lower bound on
//                                                   an edge to a sink
//   min-cost version                                same reduction, MCMF
//                                                   instead of Dinic; the free
//                                                   lo flow contributes
//                                                   lo * cost to the answer
//
// NOT THIS:
//   no lower bounds  -> plain max flow, graph/flows/Dinics.cpp.
//   "at most" only    -> that is just a capacity.
//   the lower bound is on a VERTEX -> split the vertex, put the bound on the
//     internal edge.
// ============================================================================
// needs a max-flow with add_edge(u, v, cap) and flow() -- graph/flows/Dinics.cpp

struct LowerBound {
    int n;                                  // real vertices 0..n-1
    int SS, TT;                             // the two extra vertices
    Dinic din;
    vector<ll> excess;
    vector<int> lo_of;                      // lower bound per original edge
    vector<int> id_of;                      // Dinic edge id per original edge
    ll need = 0;                            // total positive excess

    LowerBound(int n_) : n(n_), SS(n_), TT(n_ + 1), din(n_ + 2, n_, n_ + 1),
                         excess(n_ + 2, 0) {}

    // an edge u->v that must carry between lo and hi
    void add_edge(int u, int v, ll lo, ll hi) {
        lo_of.push_back(lo);
        id_of.push_back(din.m);             // the id add_edge is about to use
        din.add_edge(u, v, hi - lo);        // only the SLACK is routable
        excess[v] += lo;                    // v receives lo for free
        excess[u] -= lo;                    // u sends lo for free
    }
    // call once, after every add_edge. true iff a feasible circulation exists.
    bool feasible() {
        for (int v = 0; v < n; v++) {
            if (excess[v] > 0) din.add_edge(SS, v, excess[v]), need += excess[v];
            else if (excess[v] < 0) din.add_edge(v, TT, -excess[v]);
        }
        return din.flow() == need;          // SATURATED, not merely positive
    }
    // the actual flow on original edge i
    ll flow_on(int i) { return lo_of[i] + din.edges[id_of[i]].flow; }
};

////////////////////////////////////////////////////////////////////////////////
// FEASIBLE s-t FLOW. Add t->s with capacity INF, run feasibility, then either
// maximise or minimise. The value of the s-t flow is the flow on that edge.
//
//   LowerBound L(n);
//   ... L.add_edge(u, v, lo, hi) for every real edge ...
//   int back = L.lo_of.size();  L.add_edge(t, s, 0, INF);
//   if (!L.feasible()) -> no valid flow at all
//   ll val = L.flow_on(back);                 // a feasible s-t flow value
//
//   MAX: run din.flow() again with s and t as the terminals, add the result.
//   MIN: zero the t->s edge, run din.flow() with t as source and s as sink,
//        subtract the result.
////////////////////////////////////////////////////////////////////////////////
