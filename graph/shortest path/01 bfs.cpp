// ============================================================================
// 0-1 BFS -- shortest path when every weight is 0 or 1, O(V + E)
// ----------------------------------------------------------------------------
// WHY IT WORKS: the deque never holds more than two distinct distances, d and
//   d+1, and always in nondecreasing order. A 0-edge keeps the distance, so it
//   goes to the FRONT; a 1-edge makes it d+1, so it goes to the BACK. That is
//   the whole idea -- the deque IS the priority queue, for free.
//
// WHEN: weights in {0, 1}. Also {0, k} for a single fixed k -- the invariant is
//   the same. Beyond that see GENERALISING below.
//
// THE REAL SKILL is recognising the disguise. Cost is almost never written as
// 0/1 in the statement; it is "the number of times you do the special thing":
//   grid, turning costs 1, going straight is free   -> node = (cell, direction)
//   flip the fewest edges to make s reach t         -> forward 0, reversed 1
//   break the fewest walls to cross the grid        -> empty 0, wall 1
//   fewest portals / teleports / recharges used     -> normal move 0, portal 1
//   fewest colour changes along the path            -> node = (v, last colour)
//   fewest "buy a ticket" moves                     -> free edge 0, paid 1
// If you can phrase the answer as "how many X did I use", it is 0-1 BFS.
//
// PITFALLS:
//   NO visited[] array. Correctness comes from the `d[u] + w < d[v]` check.
//     Marking visited on PUSH is the classic wrong version -- a node can be
//     reached again later at a smaller distance through a 0-edge.
//   A node may enter the deque more than once. That is fine and still O(V+E);
//     each entry is popped once and only improves d.
//   push_front for 0, push_back for 1. Swapping them silently gives BFS order
//     and a wrong answer only on graphs where a 0-edge matters -- so it can
//     pass the samples.
//   d fits in int here (a path has at most V edges of weight 1), unlike in
//     Dijkstra where it must be ll.
//
// GENERALISING:
//   weights {0, 1}          this file
//   weights {0, k}          this file unchanged
//   weights {1, 2}          subdivide each 2-edge with a dummy node, then 0-1
//   weights 0..C, C small   Dial's algorithm, C+1 buckets, O(V*C + E)
//   anything else           Dijkstra, ./dijkstra.cpp
//
// Every graph rewrite from dijkstra.cpp still applies -- reverse the edges for
// "all nodes -> t" [R1], seed several sources at 0 [R2], expand the state [R3].
// ============================================================================
const int N = 200'123, INF = 1e9;
vector<pair<int, int>> g[N];   // (to, w) with w in {0, 1}
int d[N], par[N];

void bfs01(int s, int n) {
    for (int i = 1; i <= n; i++) d[i] = INF, par[i] = -1;
    deque<int> q;
    d[s] = 0, q.push_back(s);
    while (q.size()) {
        int u = q.front(); q.pop_front();
        // vis[] is OPTIONAL and only legal here, on POP: the deque is monotone,
        // so the first pop of u is already final. Saves rescanning g[u] on dense
        // graphs; never mark on push.   if (vis[u]) continue; vis[u] = 1;
        for (auto &[v, w] : g[u])
            if (d[u] + w < d[v]) {
                d[v] = d[u] + w, par[v] = u;
                w ? q.push_back(v) : q.push_front(v);
            }
    }
}
