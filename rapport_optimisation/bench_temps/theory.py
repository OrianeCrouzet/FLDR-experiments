"""Exact expected numbers of random bits, for the section on time optimality.

FDR(n): Lumbroso's Fast Dice Roller, as in fo_uniform (flip_opt.h).
KY(n):  Knuth-Yao optimal tree for the uniform law on n values.
"""
from fractions import Fraction
from math import log2
import sys


def fdr_bits(n):
    # State v (bound) at the top of the loop, c uniform on [0, v).
    # E = sum_k prod_{j<k} a_j along the deterministic path of v.
    E, prod, v = 0.0, 1.0, 1
    while prod > 1e-18:
        E += prod
        v = 2 * v
        if v >= n:
            prod *= 1 - n / v
            v -= n
    return E


def ky_bits(n, digits=400):
    # n leaves of probability 1/n: sum_k k d_k(1/n) 2^-k, times n.
    r, s = 1, 0.0
    for k in range(1, digits):
        r *= 2
        d, r = divmod(r, n)
        s += k * d * 2.0 ** -k
        if r == 0:
            break
    return n * s


def h(p):
    return 0 if p in (0, 1) else -p * log2(p) - (1 - p) * log2(1 - p)


if __name__ == '__main__':
    for n in [3, 5, 6, 7, 10, 100, 1000, 20484, 20485, 2**14 + 1, 3 * 2**13]:
        print(f'n={n:6d} log2n={log2(n):8.4f} FDR={fdr_bits(n):8.4f} KY={ky_bits(n):8.4f}')
    if len(sys.argv) > 1:
        # Alias table of an .alias.integers_opt file: E[h(T_i/cs)] over the q cells.
        sys.path.insert(0, '.')
        lines = open(sys.argv[1]).read().split('\n')
        t = lines[1].split()
        n = int(t[0])
        w = [int(x) for x in t[1:n + 1]]
        W = sum(w)
        cs = W // n
        q, r = divmod(W, cs)
        D = list(w)
        if r:
            D.append(cs - r)
            q += 1
        large = [i for i, x in enumerate(D) if x > cs]
        small = [i for i, x in enumerate(D) if x <= cs]
        H2, cells = 0.0, 0
        while large:
            x = large.pop()
            if small:
                x2 = small.pop()
                if D[x2] != cs:
                    H2 += h(D[x2] / cs)
                    D[x] -= cs - D[x2]
                cells += 1
            else:
                D[x] -= cs
                cells += 1
            (large if D[x] > cs else small).append(x)
        cells += len(small)
        Hp = -sum(float(Fraction(x, W)) * log2(float(Fraction(x, W))) for x in w if x)
        print(f'q={q} cells={cells} H(p)={Hp:.4f} log2 q={log2(q):.4f} '
              f'E[h(T/cs)]={H2 / q:.4f} FDR(q)={fdr_bits(q):.4f} reject={float(Fraction(cs - r, q * cs)) if r else 0:.2e}')
