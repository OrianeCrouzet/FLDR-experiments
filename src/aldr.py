# src/aldr.py

"""
ALDR (version Python)
Reprend la logique du code C aldr
"""

import numpy as np
import random

def aldr_preprocess(p_target):
    """
    Prépare les structures pour l'échantillonnage ALDR
    
    p_target : liste d'entiers (poids)
    Retourne :
      length_breadths : longueur de breadths
      breadths : liste des largeurs par niveau
      length_leaves_flat : longueur de leaves_flat
      leaves_flat : liste des feuilles aplaties
    """
    # Assume p_target is list of ints (weights)
    a = p_target
    n = len(a)
    m = sum(a)
    if m == 0:
        raise ValueError("Total weight is zero")
    k = m.bit_length() - (1 if bin(m).count('1') == 1 else 0)
    kmul = 2
    K = k * kmul  # depth
    c = (1 << K) // m  # amplification factor
    r = (1 << K) % m  # reject weight

    num_leaves = bin(r).count('1')
    for weight in a:
        num_leaves += bin(c * weight).count('1')

    breadths = [0] * (K + 1)
    leaves_flat = [0] * num_leaves

    location = 0
    for j in range(K + 1):
        bit = 1 << (K - j)
        if r & bit:
            leaves_flat[location] = 0
            breadths[j] += 1
            location += 1
        for i in range(n):
            Qi = c * a[i]
            if Qi & bit:
                leaves_flat[location] = i + 1
                breadths[j] += 1
                location += 1

    return K + 1, breadths, num_leaves, leaves_flat

def flip():
    return random.randint(0, 1)

def aldr_sample(length_breadths, breadths, length_leaves_flat, leaves_flat):
    """
    Tire un indice à partir des structures ALDR
    """
    while True:
        depth = 0
        location = 0
        val = 0
        while True:
            if val < breadths[depth]:
                ans = leaves_flat[location + val]
                if ans:
                    return ans - 1
                else:
                    break
            location += breadths[depth]
            val = ((val - breadths[depth]) << 1) | flip()
            depth += 1