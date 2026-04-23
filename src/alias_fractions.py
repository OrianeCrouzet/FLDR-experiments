# src/alias_fractions.py

from fractions import Fraction

def alias_fractions_preprocess(weights):
    """
    weights : liste d'entiers

    Retour :
        sampler = structure prête à sampler
    """
    total = sum(weights)
    distrib = [Fraction(w, total) for w in weights]

    N = len(weights)
    pdsCase = Fraction(1, N)

    S0 = []
    S1 = []

    for i in range(N):
        if distrib[i] > pdsCase:
            S0.append(i)
        else:
            S1.append(i)

    T = []

    idx0 = 0
    idx1 = 0

    for _ in range(N):

        if (len(S0) - idx0 == 0) and (len(S1) - idx1 == 0):
            raise RuntimeError("Plus de lourds ni légers")

        if len(S1) - idx1 > 0:

            moins = S1[idx1]

            if distrib[moins] == pdsCase:

                T.append({
                    "i": moins,
                    "j": -1,
                    "prob": Fraction(1, 1)
                })

                idx1 += 1

            else:
                plus = S0[idx0]

                T.append({
                    "i": moins,
                    "j": plus,
                    "prob": distrib[moins] / pdsCase
                })

                idx1 += 1

                diff = pdsCase - distrib[moins]
                distrib[plus] -= diff
                distrib[moins] = Fraction(0, 1)

                if distrib[plus] <= pdsCase:
                    idx1 -= 1
                    S1[idx1] = plus
                    idx0 += 1

        else:

            plus = S0[idx0]

            T.append({
                "i": plus,
                "j": -1,
                "prob": Fraction(1, 1)
            })

            distrib[plus] -= pdsCase

            if distrib[plus] <= pdsCase:
                idx1 -= 1
                S1[idx1] = plus
                idx0 += 1

    return {
        "taille": N,
        "table": T
    }