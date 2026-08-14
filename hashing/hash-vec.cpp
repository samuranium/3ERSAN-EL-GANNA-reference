#include <bits/stdc++.h>
#define el '\n'
#define ll long long
#define ld long double
#define fio ios_base::sync_with_stdio(0); cin.tie(0); cout.tie(0);
const int mod= 1e9 +7, N = 2e3+4;
using namespace std;
const int B = 2;
const int b[] = {200003, 200009};
int pw[B][N], inv[B][N], invB[B];
int r[B];
#define multihash array<int, B>
int add(int a, int b) { return (a+=b) < mod? a : a-mod; }
int subt(int a, int b) { return (a-=b) < 0? a + mod : a; }
int mult(int a, int b) { return 1ll*a*b%mod; }
int fp(int b, int e) {
  int r = 1;
  for(; e; b = mult(b, b), e>>=1)
    if(e&1) r = mult(r, b);
  return r;
}
mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());
void pre() {
    for (int i = 0; i < B; ++i) {
        pw[i][0] = inv[i][0] = 1;
        invB[i] = fp(b[i], mod-2);
        r[i] = rng() % (mod - 1) + 1; // salt in [1, mod-1]; rand() is 15-bit on MinGW
    }
    for (int i = 1; i < N; ++i)
        for (int base = 0; base < B; ++base)
            pw[base][i] = mult(pw[base][i-1], b[base]), inv[base][i] = mult(inv[base][i-1], invB[base]);
}
struct multisethash {
    vector<multihash> h;
    vector<multihash> hinv;

    vector<int> v;
    int n;
    multisethash(vector<int> &v) {
        this -> v = v;
        n = v.size();
        h.resize(n + 1);
        hinv.resize(n + 1);
        for (int base = 0; base < B; ++base)
            h[0][base] = 1, hinv[0][base] = 1;
        for (int i = 1; i <= n; ++i) {
            int cur = v[i - 1];
            for (int j = 0; j < B; ++j) {
                h[i][j] = mult(h[i - 1][j], add(cur, r[j]));
                hinv[i][j] = fp(h[i][j], mod-2);
            }
        }
    }
    multihash get_hash(int l, int r) {
        multihash ret;
        for (int i = 0; i < B; ++i) {
            ret[i] = mult(h[r][i], hinv[l-1][i]);
        }
        return ret;
    }
};
struct Hash {
    vector<multihash> h;
    vector<int> s;
    int n;
    int hash_num(int x, int i, int base) {
        return mult(x, pw[base][i]);
    }
    Hash(vector<int> &s) {
        this -> s = s;
        n = s.size();
        h.resize(n+1);
        for (int i = 1; i <= n; i++)
            for (int base = 0; base < B; ++base)
                h[i][base] = add(h[i-1][base], hash_num(s[i-1], i, base));
    }
    multihash get_hash(int l, int r) {
        multihash ret;
        for (int base = 0; base < B; ++base) {
            ret[base] = mult(subt(h[r][base], h[l-1][base]), inv[base][l-1]);
        }
        return ret;
    }
};
void solve() {
    int n;
    cin >> n;
    vector<pair<int, int>> a(n);
    vector<int>b;
    for (auto &[f, s] : a) {
        cin >> f >> s;
        if (f > s)
            swap(f, s);
        b.push_back((f << 13) | s);
    }
    multisethash hshb(b);
    map<multihash, int> s;
    int cnt{};

    for (int i = 1; i <= n; i++) {
        for (int j = i; j <= n; j++) {
            auto hsh = hshb.get_hash(i, j);
            s[hsh]++;
        }
    }
    for (auto &[hsh, nm]:s)
        cnt += (nm * nm - nm >> 1);
    cout << cnt << endl;
}
signed main() { /* B, N, pre()*/ fio pre();  int t = 1; /*cin >> t;*/ while (t--) solve(); return 0; }