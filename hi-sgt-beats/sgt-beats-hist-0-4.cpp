#include<algorithm>
#include<random>
#include<cassert>
#include<iostream>
using namespace std;
using ll = long long;

// Segment Tree Beats (Historic Information)
// - l<=i<r について a_i の値に x を加える
// - l<=i<r の中の a_i の最大値を求める
// - l<=i<r の中の b_i の総和を求める
// - l<=i<r の中の b_i の最大値を求める
// - (各クエリ後、全てのiについて b_i = max(a_i, b_i))

#define N 10003

class SegmentTree {
  static const ll inf = 1e18;
  int n0;

  // cur_s : a_i の区間総和
  // cur_ma: a_i の区間最大値
  ll cur_s[4*N], cur_ma[4*N];

  // 区間加算クエリ用
  ll ladd[4*N], len[4*N];

  // d_i の最小値まわりの情報を持つ構造体
  struct MinVal {
    //  min_v: d_i の最小値
    // smin_v: 二番目の最小値
    //  min_c: 最小値の個数
    //    sum: d_iの総和
    ll min_v, smin_v, min_c, sum;
    //  m_hmax: d_i が最小値の a_i + d_i
    // nm_hmax: d_i が非最小値の a_i + d_i
    ll m_hmax, nm_hmax;


    // a_i の初期値が x の初期化処理
    void init(ll x) {
      min_v = 0; smin_v = inf; min_c = 1; sum = 0;
      m_hmax = x; nm_hmax = -inf;
    }

    // a_i を使わない場合の初期化処理
    void init_empty() {
      min_v = smin_v = inf; min_c = 0; sum = 0;
      m_hmax = nm_hmax = -inf;
    }

    // d_i の最小値を x に更新
    inline void update_min(ll x) {
      if(min_v < x) {
        sum += (x - min_v) * min_c;
        m_hmax += (x - min_v);

        min_v = x;
      }
    }

    // 2つのノード l と r からこのノード情報を更新
    // (片方に自身を指定して使っても問題ないようになっている)
    inline void merge(MinVal &l, MinVal &r) {
      sum = l.sum + r.sum;
      nm_hmax = max(l.nm_hmax, r.nm_hmax);

      if(l.min_v < r.min_v) {
        smin_v = min(l.smin_v, r.min_v);
        min_v = l.min_v;
        min_c = l.min_c;

        nm_hmax = max(nm_hmax, r.m_hmax);
        m_hmax = l.m_hmax;
      } else if(l.min_v > r.min_v) {
        smin_v = min(l.min_v, r.smin_v);
        min_v = r.min_v;
        min_c = r.min_c;

        nm_hmax = max(nm_hmax, l.m_hmax);
        m_hmax = r.m_hmax;
      } else {
        min_v = l.min_v;
        smin_v = min(l.smin_v, r.smin_v);
        min_c = l.min_c + r.min_c;

        m_hmax = max(l.m_hmax, r.m_hmax);
      }
    }

    // 最小値・二番目の最小値への加算処理
    void add(ll x) {
      if(min_v != inf) min_v += x;
      if(smin_v != inf) smin_v += x;
    }

    // a_i + d_i の最大値を返す
    ll hmax() const {
      return max(m_hmax, nm_hmax);
    }
  };

  MinVal val_d[4*N];

  void addall(int k, ll a) {
    cur_s[k] += a * len[k];
    cur_ma[k] += a;
    //val_d[k].m_hmax += a; val_d[k].nm_hmax += a;

    val_d[k].add(-a);
    val_d[k].sum -= a * len[k];
    //val_d[k].m_hmax -= a; val_d[k].nm_hmax -= a;

    ladd[k] += a;
  }

  void push(int k) {
    if(k >= n0-1) return;

    if(ladd[k] != 0) {
      addall(2*k+1, ladd[k]);
      addall(2*k+2, ladd[k]);
      ladd[k] = 0;
    }

    val_d[2*k+1].update_min(val_d[k].min_v);
    val_d[2*k+2].update_min(val_d[k].min_v);
  }

  void update(int k) {
    cur_s[k] = cur_s[2*k+1] + cur_s[2*k+2];
    cur_ma[k] = max(cur_ma[2*k+1], cur_ma[2*k+2]);

    val_d[k].merge(val_d[2*k+1], val_d[2*k+2]);
  }

  // (内部用) d_i <- max(d_i, 0) で更新
  void _update_dmax(int k, int l, int r) {
    if(l == r || 0 <= val_d[k].min_v) {
      return;
    }
    if(0 < val_d[k].smin_v) {
      val_d[k].update_min(0);
      return;
    }

    push(k);
    _update_dmax(2*k+1, l, (l+r)/2);
    _update_dmax(2*k+2, (l+r)/2, r);
    update(k);
  }

  // 区間[a, b) の a_i に x を加算する
  void _add_val(ll x, int a, int b, int k, int l, int r) {
    if(b <= l || r <= a) {
      return;
    }
    if(a <= l && r <= b) {
      addall(k, x);
      _update_dmax(k, l, r);
      return;
    }
    push(k);
    _add_val(x, a, b, 2*k+1, l, (l+r)/2);
    _add_val(x, a, b, 2*k+2, (l+r)/2, r);
    update(k);
  }

  // 区間[a, b) における a_i の区間最大値を返す
  ll _query_max(int a, int b, int k, int l, int r) {
    if(b <= l || r <= a) {
      return -inf;
    }
    if(a <= l && r <= b) {
      return cur_ma[k];
    }
    push(k);
    ll lv = _query_max(a, b, 2*k+1, l, (l+r)/2);
    ll rv = _query_max(a, b, 2*k+2, (l+r)/2, r);
    return max(lv, rv);
  }

  // 区間[a, b) における historic maximal value の区間最大値を返す
  ll _query_hmax_max(int a, int b, int k, int l, int r) {
    if(b <= l || r <= a) {
      return -inf;
    }
    if(a <= l && r <= b) {
      return val_d[k].hmax();
    }
    push(k);
    ll lv = _query_hmax_max(a, b, 2*k+1, l, (l+r)/2);
    ll rv = _query_hmax_max(a, b, 2*k+2, (l+r)/2, r);
    return max(lv, rv);
  }

  // 区間[a, b) における historic maximal value の区間総和を返す
  ll _query_hmax_sum(int a, int b, int k, int l, int r) {
    if(b <= l || r <= a) {
      return 0;
    }
    if(a <= l && r <= b) {
      return cur_s[k] + val_d[k].sum;
    }
    push(k);
    ll lv = _query_hmax_sum(a, b, 2*k+1, l, (l+r)/2);
    ll rv = _query_hmax_sum(a, b, 2*k+2, (l+r)/2, r);
    return lv + rv;
  }

public:
  SegmentTree(int n, ll *a) {
    n0 = 1;
    while(n0 < n) n0 <<= 1;

    len[0] = n0;
    for(int i=0; i<2*n0-1; ++i) len[2*i+1] = len[2*i+2] = (len[i] >> 1);

    for(int i=0; i<n; ++i) {
      cur_s[n0-1+i] = cur_ma[n0-1+i] = a[i];
      val_d[n0-1+i].init(a[i]);
    }
    for(int i=n; i<n0; ++i) {
      cur_s[n0-1+i] = cur_ma[n0-1+i] = 0;
      val_d[n0-1+i].init_empty();
    }
    for(int i=n0-2; i>=0; i--) update(i);
  }

  // l<=i<r について a_i の値を a_i + x に更新
  void add_val(int a, int b, ll x) {
    _add_val(x, a, b, 0, 0, n0);
  }

  // l<=i<r の中の b_i の区間総和
  ll query_hmax_sum(int a, int b) {
    return _query_hmax_sum(a, b, 0, 0, n0);
  }

  // l<=i<r の中の b_i の区間最大値
  ll query_hmax_max(int a, int b) {
    return _query_hmax_max(a, b, 0, 0, n0);
  }

  // l<=i<r の中の a_i の区間最大値
  ll query_max(int a, int b) {
    return _query_max(a, b, 0, 0, n0);
  }
};

ll v[N], w[N];

int main() {
  random_device rnd;
  mt19937 mt(rnd());
  uniform_int_distribution<> szrnd(100, 1000);
  int n = szrnd(mt);
  uniform_int_distribution<int> rtype(0, 3), gen(0, n);
  uniform_int_distribution<ll> val(-1e4, 1e4);

  for(int i=0; i<n; ++i) v[i] = w[i] = val(mt);

  SegmentTree stb(n, v);
  int a, b;
  ll x, r0, r1;
  while(1) {
    a = gen(mt); b = gen(mt);
    x = val(mt);
    if(a == b) continue;
    if(a > b) swap(a, b);
    switch(rtype(mt)) {
      case 0:
        //cout << "add_val (" << a << ", " << b << ") : " << x << endl;
        stb.add_val(a, b, x);
        for(int i=a; i<b; ++i) {
          v[i] += x;
          w[i] = max(v[i], w[i]);
        }
        break;
      case 1:
        r0 = stb.query_max(a, b);
        r1 = -1e18;
        for(int i=a; i<b; ++i) {
          r1 = max(r1, v[i]);
        }
        if(r0 != r1) {
          cout << "query max (" << a << ", " << b << ") : " << r0 << " " << r1 << endl;
        }
        break;
      case 2:
        r0 = stb.query_hmax_sum(a, b);
        r1 = 0;
        for(int i=a; i<b; ++i) {
          r1 += w[i];
        }
        if(r0 != r1) {
          cout << "query hmax sum (" << a << ", " << b << ") : " << r0 << " " << r1 << endl;
        }
        break;
      case 3:
        r0 = stb.query_hmax_max(a, b);
        r1 = -1e18;
        for(int i=a; i<b; ++i) {
          r1 = max(r1, w[i]);
        }
        if(r0 != r1) {
          cout << "query hmax max (" << a << ", " << b << ") : " << r0 << " " << r1 << endl;
        }
        break;
      default:
        continue;
    }
  }
  return 0;
}
