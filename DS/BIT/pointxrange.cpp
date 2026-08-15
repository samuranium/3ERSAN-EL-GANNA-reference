template <class T>
struct BIT { //1-indexed
    int n; vector<T> t;
    BIT() {}
    BIT(int _n) {
        n = _n; t.assign(n + 1, 0);
    }
    T query(int i) {
        T ans = 0;
        for (; i >= 1; i -= (i & -i)) ans += t[i];
        return ans;
    }
    void upd(int i, T val) {
        if (i <= 0) return;
        for (; i <= n; i += (i & -i)) t[i] += val;
    }
    void upd(int l, int r, T val) {
        upd(l, val);
        upd(r + 1, -val);
    }
    T query(int l, int r) {
        return query(r) - query(l - 1);
    }
};
/////////////////////////////////////////////////////
template <class T>
struct FenwickTree {
public:
  FenwickTree() : n(0) {}
  FenwickTree(int _n) : n(_n), data(_n, 0) {}

  void add(int p, T x) {
    p++;
    while (p <= n) {
      data[p - 1] += x;
      p += p & -p;
    }
  }

  // [l, r) (0-indexed)
  T sum(int l, int r) {
    return sum(r) - sum(l);
  }

  // lower_bound on prefix sum
  int lower_bound(int k) {
    int pos = 0;
    for (int i = 20; i >= 0; i--) { // change 20 as N changes
      if (pos + (1 << i) <= n && data[pos + (1 << i) - 1] < k) {
        pos += (1 << i);
        k -= data[pos - 1];
      }
    }
    return pos;
  }

private:
  int n;
  std::vector<T> data;

  T sum(int r) {
    T s = 0;
    while (r > 0) {
        s += data[r - 1];
        r -= r & -r;
    }
    return s;
  }
};
