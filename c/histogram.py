import numpy as np
import matplotlib.pyplot as plt

N = 50

# --- lecture distribution ---
with open("histogram/fractions.dist") as f:
    Z = int(f.readline())
    line = f.readline().split()
    n = int(line[0])
    weights = np.array(list(map(int, line[1:])))

# --- lecture tirages ---
with open("histogram/alias.fractions.histo") as f:
    draws = np.array(list(map(int, f.read().split())))
draws = np.array(draws)
# on ignore l'objet virtuel
draws = draws[draws < n]

# Alias exact
#draws = draws[(draws >= 1) & (draws <= n)]
#counts = np.bincount(draws, minlength=n+1)[1:]

# --- histogramme empirique ---
counts = np.bincount(draws, minlength=n)
counts = counts[:n]

# --- normalisation pour comparaison ---
weights_norm = weights / weights.sum()
counts_norm = counts / counts.sum()

# --- plot ---
x = np.arange(n)    # les objets de 0 à n-1
# Alias Exact
#x = np.arange(1, n+1)   # les objets de 1 à n

plt.figure(figsize=(12,5))

plt.bar(x, weights_norm, alpha=0.5, color="pink", label="distribution théorique")
plt.bar(x, counts_norm, alpha=0.5, color="blue", label="tirages (empirique)")

plt.legend()
plt.title("Validation sampler : Alias Fractions")
plt.show()