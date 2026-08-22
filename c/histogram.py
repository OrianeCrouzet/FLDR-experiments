import numpy as np
import matplotlib.pyplot as plt

# --- 1. Lecture distribution (Poids GMP) ---
with open("histogram/test6.dist") as f:
    Z = int(f.readline())
    line = f.readline().split()
    n = int(line[0])
    weights_raw = [int(x) for x in line[1:]]

# --- 2. Lecture tirages ---
with open("histogram/alias.rust_gmp.histo") as f:
    # On force la conversion en C-int64 standard pour np.bincount (très rapide)
    draws_raw = list(map(int, f.read().split()))

# Convertit en uint32/int64 NumPy
draws = np.array(draws_raw, dtype=np.int64)

# Décalage si vos tirages sont indexés de 1 à n (ici min=1, max=20474)
if draws.min() == 1:
    draws = draws - 1

# --- 3. Comptage empirique ---
counts = np.bincount(draws, minlength=n)[:n]

# --- 4. Normalisation ---
total_weights = sum(weights_raw)
weights_norm = np.array([w / total_weights for w in weights_raw], dtype=float)
counts_norm = counts / counts.sum()

# --- 5. Visualisation ---
fig, axes = plt.subplots(2, 1, figsize=(12, 8))

# A. Vue globale agrégée (100 bins pour rendre les variations lisibles)
num_bins = 100
counts_binned, bin_edges = np.histogram(draws, bins=num_bins, range=(0, n))
weights_binned = np.histogram(np.arange(n), bins=num_bins, weights=weights_norm, range=(0, n))[0]

bin_centers = (bin_edges[:-1] + bin_edges[1:]) / 2

axes[0].bar(bin_centers, weights_binned, width=n/num_bins, alpha=0.4, color="pink", label="Théorique (agrégé)")
axes[0].step(bin_centers, counts_binned / counts_binned.sum(), where='mid', color="blue", label="Empirique (1M tirages)")
axes[0].set_title(f"Vue globale agrégée sur {num_bins} paquets (n = {n})")
axes[0].set_ylabel("Densité de probabilité")
axes[0].legend()
axes[0].grid(True, alpha=0.3)

plt.tight_layout()
plt.show()