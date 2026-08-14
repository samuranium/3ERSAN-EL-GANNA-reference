#include <bits/stdc++.h>
typedef long long ll;
typedef long double ld;
#define el '\n'
#define fio ios_base::sync_with_stdio(0); cin.tie(0); cout.tie(0);
#define mat vector<vector<ll>>
#define zero(n,m) mat(n , vector<ll>(m,0))
const int mod = 1e9;
using namespace std;
mat mult(mat &a , mat &b){
  int n = a.size() , m = b[0].size();
  mat res = zero(n , m);
  for(auto &i : res)
    for(auto &j : i)
      j = 4e18;
  for(int i = 0; i < n; i++) 
  for(int j = 0; j < m; j++) 
  for(int k = 0; k < n; k++) 
    res[i][j] = min(res[i][j], a[i][k] + b[k][j]);
  return res;
}
mat fp(mat &b , ll e){
  mat r = zero(b.size(), b.size());
  for(; e; b = mult(b, b), e >>= 1)
    if(e & 1) r = mult(r, b);
  return r;
}
const int N = 112;
void solve(){ 
  int n, m, k;
  cin >> n >> m >> k;
  mat g = zero(n+1, n+1);
  for(auto &i : g)
    for(auto &j : i) j = 4e18;
  for(int u, v,c, i = 0; i < m; i++){
    cin >> u >> v >> c;
    g[u][v] = c;
  }
  g = fp(g, k);
  ll out = 4e18;
  for(int i = 1; i <= n; i++){
    for(int j = 1; j <= n; j++){
      out = min(out, g[i][j]);
    }
  }
  if(out > 1e18) 
    cout << "IMPOSSIBLE\n";
  else 
    cout << out << el;
}
int main(){ fio int t = 1; /*cin >> t;*/ while(t--) solve(); return 0; }














