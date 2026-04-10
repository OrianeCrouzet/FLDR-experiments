#!/usr/bin/env python3

"""
Generate families of discrete integer distributions for sampling benchmarks.

Output format (.dist file):

Line 1 : Z (common denominator)
Line 2 : n followed by n integer numerators (sum = Z)
Line 3 : entropy of the distribution

Usage:

python3 create_distributions.py n Z m --seed 42

Example:

python3 create_distributions.py 1000 10000 50

This will generate 50 distributions per family.

Output directories are created under experiments/distributions as:
    dists.custom.{family}.{n}.{Z}.{seed}
"""

import os
import shutil
import numpy as np
from scipy import stats
import argparse

BASE_DIR = os.path.dirname(os.path.realpath(__file__))
DISTRIBUTIONS_DIR = os.path.join(BASE_DIR, 'distributions')


# Shannon entropy
def compute_entropy(weights):
    probs = weights / np.sum(weights)
    # avoid log(0)
    probs = probs[probs > 0]
    return -np.sum(probs * np.log2(probs))


# Normalize real weights to integer weights summing exactly to Z
# Ensure every element is at least 1.
def normalize_to_Z(weights, Z):
    weights = np.maximum(weights, 0)
    if np.sum(weights) == 0:
        weights = np.ones(len(weights))

    n = len(weights)
    if Z < n:
        raise ValueError("Z must be at least n to assign at least 1 to each element")

    weights = weights / np.sum(weights)
    scaled = weights * (Z - n)
    integers = np.floor(scaled).astype(int) + 1
    diff = Z - np.sum(integers)

    # distribute remainder deterministically
    if diff > 0:
        frac = scaled - np.floor(scaled)
        idx = np.argsort(-frac)
        integers[idx[:diff]] += 1

    return integers


# Distribution families

def uniform_family(n):
    return np.ones(n)


def dirichlet_family(n):
    alpha = 10**np.random.uniform(-2,1) # Pour faire varier l'entropie
    return np.random.dirichlet(np.ones(n)*alpha)


def sparse_family(n):
    weights = np.zeros(n)
    # Choisir aléatoirement k et affecter des poids aléatoires seulement à ces k positions
    # Le reste est constitué de 1
    k = np.random.randint(1,n//2)   # pour faire varier l'entropie
    idx = np.random.choice(n, k, replace=False)
    weights[idx] = np.random.random(k)
    return weights


def gaussian_family(n):
    x = np.arange(n)
    # Moyenne aléatoire et écart-type aléatoire pour faire varier l'entropie
    mu = np.random.uniform(0, n)
    sigma = 10**np.random.uniform( 
        np.log10(n/100),
        np.log10(n/2)
    )
    return np.exp(-(x-mu)**2/(2*sigma**2))


def exponential_family(n):
    # Décroissance exponentielle
    x = np.arange(n)
    lam = 10**np.random.uniform(    # pour faire varier l'entropie
        np.log10(1/n),
        np.log10(10/n)
    )
    return np.exp(-lam*x)


def powerlaw_family(n):
    x = np.arange(1, n+1)
    alpha = np.random.uniform(0.5,4)  # pour faire varier l'entropie
    return 1/(x**alpha)


def dominant_family(n):
    # Premier élément dominant, les autres sont égaux
    weights = np.ones(n)
    dominance = 10**np.random.uniform(1,5)
    weights[0] = dominance
    return weights


# Families list
FAMILIES = {
    "uniform": uniform_family,
    "dirichlet": dirichlet_family,
    "sparse": sparse_family,
    "gaussian": gaussian_family,
    "exponential": exponential_family,
    "powerlaw": powerlaw_family,
    "dominant": dominant_family
}


def write_distribution(path, idx, numerators, Z):
    n = len(numerators)
    entropy = compute_entropy(numerators)
    fname = f"d.{idx:05d}.dist"
    with open(os.path.join(path, fname), "w") as f:
        f.write(f"{Z}\n")
        f.write(f"{n} ")
        f.write(" ".join(map(str, numerators)))
        f.write("\n")
        f.write(f"{entropy:.6f}\n")


def main():
    parser = argparse.ArgumentParser()

    parser.add_argument("n", type=int)
    parser.add_argument("Z", type=int)
    parser.add_argument("m", type=int)
    parser.add_argument("--seed", type=int, default=42)
    args = parser.parse_args()

    np.random.seed(args.seed)

    os.makedirs(DISTRIBUTIONS_DIR, exist_ok=True)
    print("Generating distributions")

    for name, generator in FAMILIES.items():
        print("Family:", name)
        family_dir = os.path.join(DISTRIBUTIONS_DIR, f'dists.custom.{name}.{args.n}.{args.Z}.{args.seed}')
        if os.path.exists(family_dir):
            shutil.rmtree(family_dir)
        os.makedirs(family_dir, exist_ok=True)
        for i in range(args.m):
            raw = generator(args.n)
            numerators = normalize_to_Z(raw, args.Z)
            write_distribution(family_dir, i, numerators, args.Z)

    print("Done")


if __name__ == "__main__":
    main()