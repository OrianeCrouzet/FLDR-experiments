# src/alias_integers.py

"""
Alias method pour poids entiers (version Python)
Reprend la logique du code C alias.integers
"""

import numpy as np

def alias_integers_preprocess(p_target):
    """
    Prépare la table pour l'échantillonnage Alias (version entiers)
    
    p_target : liste/array d'entiers (poids)
    Retourne :
      K : nombre d'éléments
      T : table des indices
      Threshold : table des seuils
      cs : valeur de normalisation (moyenne des poids)
    """
    n = len(p_target)
    D = list(p_target)  # copie
    T = []
    Threshold = []

    # cs = poids moyen
    total_weight = sum(D)
    cs = total_weight // n  # division entière
    virtual_obj = None
    remainder = total_weight % n

    if remainder > 0:
        virtual_obj = n
        delta = cs - remainder
        D.append(delta)
        n = len(D)

    # Initialisation des piles H et L
    H = [i for i, w in enumerate(D) if w >= cs]
    L = [i for i, w in enumerate(D) if w < cs]

    t = 0
    while H:
        x = H.pop()
        w = D[x]

        if L:
            x2 = L.pop()
            w2 = D[x2]

            if virtual_obj is not None and x2 == virtual_obj:
                virtual_cell = t
                T.extend([x, x2])
                Threshold.append(cs - w2)
            else:
                T.extend([x2, x])
                Threshold.append(w2)

            w -= cs - w2
        else:
            T.extend([x, -1])
            Threshold.append(cs)
            w -= cs

        t += 2
        if w > 0:
            D[x] = w
            if w >= cs:
                H.append(x)
            else:
                L.append(x)

    K = len(T)
    return K, T, Threshold, cs


def alias_integers_sample(K, T, Threshold, cs):
    """
    Tire un indice à partir de la table Alias
    """
    # Tirage uniforme d'une case
    q = np.random.randint(0, K // 2)  # chaque cellule correspond à 2 indices
    b = np.random.randint(0, Threshold[q] + 1) < Threshold[q]

    # Calcul de l'indice final (équivalent au C)
    final_index = 2 * q + 1 - int(b)
    return T[final_index]