#!/usr/bin/env python3
#
# Copyright 2019 MIT Probabilistic Computing Project.
# Released under Apache 2.0; refer to LICENSE.txt

import multiprocessing as mp

try:
    mp.set_start_method("fork")
except RuntimeError:
    pass

import os
import shutil
import subprocess
import math
import sys

from fractions import Fraction

import matplotlib;
matplotlib.use('Agg')

import matplotlib.pyplot as plt
import numpy as np

from create_distributions import normalize_to_Z

from discrete_sampling.construct import construct_sample_alias
from discrete_sampling.construct import construct_sample_alias_integers
from discrete_sampling.construct import construct_sample_alias_integers_old
from discrete_sampling.construct import construct_sample_alias_rust
from discrete_sampling.construct import construct_sample_alias_fractions
from discrete_sampling.construct import construct_sample_aldr
from discrete_sampling.construct import construct_sample_interval
from discrete_sampling.construct import construct_sample_ky_approx_encoding
from discrete_sampling.construct import construct_sample_ky_approx_matrix
from discrete_sampling.construct import construct_sample_ky_approx_matrix_cached
from discrete_sampling.construct import construct_sample_ky_encoding
from discrete_sampling.construct import construct_sample_ky_matrix
from discrete_sampling.construct import construct_sample_ky_matrix_cached
from discrete_sampling.construct import construct_sample_rejection_binary_search
from discrete_sampling.construct import construct_sample_rejection_encoding
from discrete_sampling.construct import construct_sample_rejection_hash_table
from discrete_sampling.construct import construct_sample_rejection_matrix
from discrete_sampling.construct import construct_sample_rejection_matrix_cached
from discrete_sampling.construct import construct_sample_rejection_uniform

from discrete_sampling.writeio import write_sample_alias
from discrete_sampling.writeio import write_sample_alias_integers
from discrete_sampling.writeio import write_sample_alias_integers_old
from discrete_sampling.writeio import write_sample_alias_rust
from discrete_sampling.writeio import write_sample_alias_fractions
from discrete_sampling.writeio import write_sample_aldr
from discrete_sampling.writeio import write_sample_interval
from discrete_sampling.writeio import write_sample_ky_encoding
from discrete_sampling.writeio import write_sample_ky_matrix
from discrete_sampling.writeio import write_sample_ky_matrix_cached
from discrete_sampling.writeio import write_sample_rejection_binary_search
from discrete_sampling.writeio import write_sample_rejection_hash_table
from discrete_sampling.writeio import write_sample_rejection_uniform

from discrete_sampling.entropy import compute_entropy
from discrete_sampling.entropy import get_alpha_entropies
from discrete_sampling.utils import get_common_denominator
from discrete_sampling.utils import get_common_numerators
from discrete_sampling.utils import sample_dirichlet_multinomial_positive

from parallel_map import parallel_map
from parsable import parsable

def get_distribution_least_entropy(n, Z):
    assert n <= Z
    S = Z - n
    numerators = [1] * n
    numerators[0] += S
    assert sum(numerators) == Z
    return [Fraction(a, Z) for a in numerators]

def get_distribution_most_entropy(n, Z):
    assert n <= Z
    numerators = np.ones(n, dtype=int)
    S = Z - n
    numerators += S // n
    numerators[:(S%n)] += 1
    assert sum(numerators) == Z
    return [Fraction(int(a), Z) for a in numerators]

def get_distribution_entropy_bounds(n, Z):
    l = get_distribution_least_entropy(n, Z)
    h = get_distribution_most_entropy(n, Z)
    el = compute_entropy(l)
    eh = compute_entropy(h)
    assert el <= eh
    return (el, eh)

def write_samplers(args):
    (samplers, dirname, idx, p_target, entropy) = args
    structures = [
        ('ky.enc',
            construct_sample_ky_encoding,
            write_sample_ky_encoding),
        ('ky.mat',
            construct_sample_ky_matrix,
            write_sample_ky_matrix),
        ('ky.matc',
            construct_sample_ky_matrix_cached,
            write_sample_ky_matrix_cached),

        ('ky.approx.enc',
            construct_sample_ky_approx_encoding,
            write_sample_ky_encoding),
        ('ky.approx.mat',
            construct_sample_ky_approx_matrix,
            write_sample_ky_matrix),
        ('ky.approx.matc',
            construct_sample_ky_approx_matrix_cached,
            write_sample_ky_matrix_cached),

        ('rej.uniform',
            construct_sample_rejection_uniform,
            write_sample_rejection_uniform),
        ('rej.table',
            construct_sample_rejection_hash_table,
            write_sample_rejection_hash_table),
        ('rej.binary',
            construct_sample_rejection_binary_search,
            write_sample_rejection_binary_search),

        ('rej.enc',
            construct_sample_rejection_encoding,
            write_sample_ky_encoding),
        ('rej.mat',
            construct_sample_rejection_matrix,
            write_sample_ky_matrix),
        ('rej.matc',
            construct_sample_rejection_matrix_cached,
            write_sample_ky_matrix_cached),

        ('interval',
            construct_sample_interval,
            write_sample_interval),
        ('alias.exact',
            construct_sample_alias,
            write_sample_alias),
        ('alias.integers',
            construct_sample_alias_integers,
            write_sample_alias_integers),
        ('alias.integers_old',
            construct_sample_alias_integers_old,
            write_sample_alias_integers_old),
        ('alias.rust',
            construct_sample_alias_rust,
            write_sample_alias_rust),
        ('aldr',
            construct_sample_aldr,
            write_sample_aldr),
        ('alias.fractions',
            construct_sample_alias_fractions,
            write_sample_alias_fractions),
    ]

    for suffix, f_construct, f_write in structures:
        if samplers and suffix not in samplers:
            continue
        fpath = os.path.join(dirname, 'd.%05d.%s' % (idx, suffix))
        struc = f_construct(p_target)
        if suffix in {'alias.integers', 'alias.integers_old', 'alias.rust', 'alias.fractions', 'aldr'}:
            f_write(*struc, entropy, fpath)
        else:
            f_write(*struc, fpath)
        print(fpath)

    fname_dist = 'd.%05d.dist' % (idx,)
    fpath_dist = os.path.join(dirname, fname_dist)
    with open(fpath_dist, 'w') as f:
        n = len(p_target)
        Z = get_common_denominator(p_target)
        Ms = get_common_numerators(Z, p_target)
        f.write('%d\n' % (Z,))
        f.write('%d %s\n' % (n, ' '.join(map(str, Ms))))
        f.write('%1.5f\n' % (entropy,))
        print(fpath_dist)

    # Make soft links to dist file for non ANCI C baselines.
    for suffix in ['alias.boost', 'inversion.std', 'alias.gsl']:
        if samplers and suffix not in samplers:
            continue
        fname_suffix = fname_dist.replace('.dist', '.%s' % (suffix,))
        fpath = os.path.join(dirname, fname_suffix)
        subprocess.check_output(['ln', fpath_dist, fpath])
        print(fpath)

@parsable
def generate_distributions(N=10, Z=-1, seed=1, samplers='', thin=1,
        force=None, offset=0):
    """Generate distributions and save to disk."""
    Z =  2*N**2 + 1 if Z == - 1 else int(Z)
    dirname = 'dists.%d.%d.%d' % (N, Z, seed,)
    samplers = samplers.replace('\'', '').split(' ') if samplers != '' else []
    if force and os.path.exists(dirname):
        shutil.rmtree(dirname)
    if not os.path.exists(dirname):
        os.mkdir(dirname)

    # XXX
    import sys; sys.setrecursionlimit(100000)

    rng = np.random.RandomState(seed)
    alphas = get_alpha_entropies(N, maxalpha=5, numalpha=1000, parallel=True)
    distributions = [
        sample_dirichlet_multinomial_positive(a, N, Z, rng)
        # for a in alphas
        for a in alphas[offset::thin]
    ]
    # dernière distribution = uniforme parfaite
    u = np.ones(N)
    weights = normalize_to_Z(u, Z)
    distributions.append([Fraction(int(w), Z) for w in weights])

    low, high = get_distribution_entropy_bounds(N, Z)
    entropies = parallel_map(compute_entropy, distributions)
    idxs = np.argsort(entropies)
    args = [
        (samplers, dirname, i, distributions[idx], entropies[idx])
        for i, idx in enumerate(idxs)
    ]
    parallel_map(write_samplers, args)
    # list(map(write_samplers, args))

@parsable
def generate_distributions_entropy2(
        H=10.0,
        maxpow=12,
        Z=-1,
        seed=1,
        samplers='',
        thin=1,
        force=None,
        offset=0):
    """
    Génère des distributions pour tous les N=2^k compatibles
    avec une entropie cible H.
    """

    sys.setrecursionlimit(100000)

    samplers = samplers.replace("'", "").split(" ") if samplers != "" else []

    kmin = math.ceil(H)
    Ns = [2**k for k in range(kmin, maxpow + 1)]

    rng = np.random.RandomState(seed)

    for N in Ns:

        ZN = 2 * N**2 + 1 if Z == -1 else int(Z)
        H_int = int(H)
        dirname = f'dists.entropy.{H_int}.{maxpow}.{seed}'

        if force and os.path.exists(dirname):
            shutil.rmtree(dirname)

        if not os.path.exists(dirname):
            os.mkdir(dirname)

        # récupère familles alpha
        alphas = get_alpha_entropies(
            N,
            maxalpha=5,
            numalpha=1000,
            parallel=True
        )

        distributions = [
            sample_dirichlet_multinomial_positive(a, N, ZN, rng)
            for a in alphas[offset::thin]
        ]

        # uniforme parfaite
        u = np.ones(N)
        weights = normalize_to_Z(u, ZN)
        distributions.append(
            [Fraction(int(w), ZN) for w in weights]
        )

        entropies = parallel_map(compute_entropy, distributions)

        # garder seulement celles proches de H
        tol = 1e-3
        filtered = [
            (d, e) for d, e in zip(distributions, entropies)
            if abs(e - H) <= tol
        ]

        if not filtered:
            continue

        idxs = np.argsort([e for _, e in filtered])

        args = [
            (samplers, dirname, i,
             filtered[idx][0],
             filtered[idx][1])
            for i, idx in enumerate(idxs)
        ]

        parallel_map(write_samplers, args)

@parsable
def generate_distributions_entropy(
        H=10.0,
        maxpow=15,
        per_N=20,
        Z=-1,
        seed=1,
        samplers='',
        thin=1,
        force=None):
    """
    Génère plusieurs distributions pour chaque N = 2^k
    avec entropie cible H (bits).

    Pour chaque N :
        k = ceil(H) ... maxpow

    Crée per_N distributions différentes.
    Compatible write_samplers().
    """

    import os
    import math
    import shutil
    import numpy as np
    from fractions import Fraction

    rng = np.random.RandomState(seed)

    samplers = samplers.replace("'", "").split(" ") if samplers != "" else []

    dirname = f'dists.entropy.{int(H)}.{maxpow}.{seed}'

    if force and os.path.exists(dirname):
        shutil.rmtree(dirname)

    if not os.path.exists(dirname):
        os.mkdir(dirname)

    # -------------------------------------------------
    # Shannon entropy (bits)
    # -------------------------------------------------
    def entropy(p):
        p = np.asarray(p, dtype=float)
        p = p[p > 0]
        return -np.sum(p * np.log2(p))

    # -------------------------------------------------
    # famille simple :
    # p = [a, (1-a)/(N-1), ..., ]
    # -------------------------------------------------
    def family_distribution(a, N):
        rest = (1.0 - a) / (N - 1)
        p = np.full(N, rest)
        p[0] = a
        return p

    # -------------------------------------------------
    # recherche dichotomique du a donnant H
    # -------------------------------------------------
    def solve_a_for_entropy(N, Htarget):
        lo = 1.0 / N
        hi = 0.999999999

        for _ in range(80):
            mid = (lo + hi) / 2
            h = entropy(family_distribution(mid, N))

            if h > Htarget:
                lo = mid
            else:
                hi = mid

        return (lo + hi) / 2

    # -------------------------------------------------
    # transforme en fractions somme = Z
    # -------------------------------------------------
    def rationalize(p, Z):
        w = np.floor(p * Z).astype(int)
        missing = Z - np.sum(w)

        frac = p * Z - w
        idx = np.argsort(-frac)

        for i in range(missing):
            w[idx[i]] += 1

        return [Fraction(int(x), Z) for x in w]

    # -------------------------------------------------
    # boucle sur N = puissances de 2
    # -------------------------------------------------
    kmin = int(math.ceil(H))

    global_index = 0

    for k in range(kmin, int(maxpow) + 1):

        N = 2 ** k
        ZN = 2 * N * N + 1 if int(Z) == -1 else int(Z)

        print("Generating N =", N, "Z =", ZN)

        # a central donnant entropie cible
        a0 = solve_a_for_entropy(N, float(H))

        distributions = []

        for j in range(int(per_N)):

            # petite perturbation aléatoire
            delta = rng.uniform(-0.03, 0.03)
            a = min(0.999999, max(1.0 / N, a0 + delta))

            p = family_distribution(a, N)

            # permutation aléatoire => distributions distinctes
            rng.shuffle(p)

            # renormalisation
            p = p / np.sum(p)

            # corrige entropie si nécessaire
            # (recherche fine sur mélange uniforme)
            h = entropy(p)

            u = np.ones(N) / N

            lam_lo = 0.0
            lam_hi = 1.0

            for _ in range(40):
                lam = (lam_lo + lam_hi) / 2
                q = lam * p + (1 - lam) * u
                hq = entropy(q)

                if hq > H:
                    lam_lo = lam
                else:
                    lam_hi = lam

            q = lam * p + (1 - lam) * u
            q = q / np.sum(q)

            dist = rationalize(q, ZN)
            h_final = compute_entropy(dist)

            distributions.append((dist, h_final))

        # tri par entropie
        distributions.sort(key=lambda x: x[1])

        args = [
            (samplers, dirname, global_index + i, d, h)
            for i, (d, h) in enumerate(distributions)
        ]

        parallel_map(write_samplers, args)

        global_index += len(distributions)

if __name__ == '__main__':
    parsable()
