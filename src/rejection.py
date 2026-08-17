# Released under Apache 2.0; refer to LICENSE.txt

from fractions import Fraction
from math import ceil
from math import log2

from numpy import cumsum

from discrete_sampling.entropy import compute_entropy
from discrete_sampling.utils import get_binary_expansion
from discrete_sampling.utils import get_common_denominator
from discrete_sampling.utils import get_common_numerators

def get_rejection_precision2(p_target):
    # Version originale
    Z = get_common_denominator(p_target)
    return ceil(log2(Z))

def get_rejection_precision(p_target):
    # En test pour les grands entiers
    Z = int(get_common_denominator(p_target))

    # Si Z est une puissance exacte de 2, bit_length() - 1 donne log2(Z)
    # Sinon, bit_length() donne exactement ceil(log2(Z))
    if (Z & (Z - 1)) == 0:
        return Z.bit_length() - 1

    return Z.bit_length()

def get_rejection_p_success(p_target):
    Z = get_common_denominator(p_target)
    k = get_rejection_precision(p_target)
    return Fraction(Z, 2**k)

def get_rejection_num_trials(p_target):
    p_success = get_rejection_p_success(p_target)
    return 1 / p_success

def get_rejection_num_bits_per_trial(p_rejection):
    return compute_entropy(p_rejection)

def get_rejection_probabilities2(p_target):
    # Version originale
    Z = get_common_denominator(p_target)
    k = get_rejection_precision2(p_target)
    numerators = get_common_numerators(Z, p_target)
    p_reject = 1 - get_rejection_p_success(p_target)
    return [Fraction(n, 2**k) for n in numerators] + [p_reject]

def get_rejection_probabilities(p_target):
    # En test pour les grands entiers
    # Dénominateur Z exact en int Python
    Z = int(get_common_denominator(p_target))
    k = get_rejection_precision(p_target)

    # 1. Numérateurs d'origine sur Z
    numerators = get_common_numerators(Z, p_target)

    # 2. Les probabilités réelles de p_target (sur Z)
    p_fracs = [Fraction(n, Z) for n in numerators]

    # 3. La probabilité totale de rejet : 1 - (1/Z) ou selon le facteur d'échelle k
    # p_reject est exactement la fraction (2^k - sum(numerators)) / 2^k
    # pour que la somme des probabilités soit exactement égale à 1 sur 2^k
    Zkl = 1 << k

    # La masse affectée aux cibles sur 2^k est le numérateur d'origine
    # Le rejet prend TOUT le reste exact pour compléter à 2^k
    reject_numerator = Zkl - sum(numerators)

    # On reconstruit les probabilités
    p_rejection = [Fraction(n, Zkl) for n in numerators]
    p_rejection.append(Fraction(reject_numerator, Zkl))

    return p_rejection

def get_rejection_Ms_k2(p_target):
    # Version originale
    # TODO: Use get_common_numerators from utils.
    p_rejection = get_rejection_probabilities2(p_target)
    k = get_rejection_precision2(p_target)
    return [int(2**k * p) for p in p_rejection], k

def get_rejection_Ms_k(p_target):
    # En test pour les grands entiers
    Z = int(get_common_denominator(p_target))
    k = get_rejection_precision(p_target)
    Zkl = 1 << k

    numerators = get_common_numerators(Z, p_target)

    # Les N premiers éléments conservent leur numérateur exact
    Ms = [int(n) for n in numerators]

    # Le dernier élément (rejet) prend la différence exacte Zkl - sum(numerators)
    reject_M = Zkl - sum(Ms)
    Ms.append(reject_M)

    assert sum(Ms) == Zkl, f"Erreur de somme: {sum(Ms)} != {Zkl}"
    return Ms, k

def get_rejection_table(p_target):
    Z = get_common_denominator(p_target)
    numerators = get_common_numerators(Z, p_target)
    T = [0] * Z
    j = 0
    for i, n in enumerate(numerators):
        T[j:j+n] = [i+1]*n
        j += n
    assert sum(1 for x in T if x > 0) == Z
    return T

def get_rejection_cdf(p_target):
    Z = get_common_denominator(p_target)
    numerators = get_common_numerators(Z, p_target)
    cdf = [0] + list(cumsum(numerators))
    return cdf

def make_rejection_ddg_matrix(p_target):
    n = len(p_target)
    Z = get_common_denominator(p_target)
    k = get_rejection_precision(p_target)
    Ms = get_common_numerators(Z, p_target)
    M_reject = (1 << k) - Z

    h = [0] * k
    H = [-1] * ((n+1)*k)

    for j in range(k):
        d = 0
        for i in range(n):
            w = (Ms[i] >> ((k-1) -j)) & 1
            h[j] += (w > 0)
            if w > 0:
                H[d*k + j] = i
                d += 1
        w = (M_reject >> ((k-1) -j)) & 1
        h[j] += (w > 0)
        if w > 0:
            H[d*k +j] = n

    return h, H
