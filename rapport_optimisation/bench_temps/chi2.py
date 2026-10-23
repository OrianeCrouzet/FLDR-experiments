"""Chi-square tests of the exact variants of bench.c.

  python3 chi2.py FILE.alias.integers_opt hist/PREFIX.*      marginal law
  python3 chi2.py FILE.alias.integers_opt --pairs SEQ.bin   successive pairs (<= 8 x 8 classes)
"""
import sys
from fractions import Fraction
import numpy as np
from scipy.stats import chi2

lines = open(sys.argv[1]).read().split('\n')
t = lines[1].split()
n = int(t[0])
w = [int(x) for x in t[1:n + 1]]
W = sum(w)
p = np.array([float(Fraction(wi, W)) for wi in w])


def test(obs, exp):
    big = exp >= 5
    o = obs[big].astype(float)
    e = exp[big]
    if (~big).any():
        o = np.append(o, obs[~big].sum())
        e = np.append(e, exp[~big].sum())
    st = ((o - e) ** 2 / e).sum()
    return st, len(o) - 1, chi2.sf(st, len(o) - 1)


if sys.argv[2] == '--pairs':
    x = np.fromfile(sys.argv[3], dtype=np.uint32)
    # 8 classes of about equal mass (objects sorted by index, cut on the cumulative law).
    cls = np.unique(np.minimum((np.cumsum(p) * 8).astype(int), 7), return_inverse=True)[1]
    k = cls.max() + 1
    pc = np.bincount(cls, weights=p, minlength=k)
    c = cls[x]
    pairs = np.bincount(c[:-1] * k + c[1:], minlength=k * k)
    exp = (len(c) - 1) * np.outer(pc, pc).ravel()
    st, df, pv = test(pairs, exp)
    print(f'{sys.argv[3]}: pairs chi2={st:.1f} df={df} p={pv:.3f}')
else:
    for f in sys.argv[2:]:
        h = open(f).read().split('\n')
        obs = np.array([int(v) for v in h[1:] if v != ''])
        N = obs.sum()
        assert obs[n:].sum() == 0
        st, df, pv = test(obs[:n], N * p)
        print(f'{h[0]:34s} N={N} chi2={st:.0f} df={df} p={pv:.3f}')
