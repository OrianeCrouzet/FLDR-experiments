# Released under Apache 2.0; refer to LICENSE.txt

def write_array(array, f):
    n = len(array)
    f.write('%d ' % (n,))
    f.write(' '.join(map(str, array)))
    f.write('\n')

def write_matrix(matrix, f):
    nrow = len(matrix)
    ncol = len(matrix[0])
    f.write('%d %d\n' % (nrow, ncol))
    for row in matrix:
        f.write(' '.join(map(str, row)))
        f.write('\n')

def write_sample_ky_encoding(enc, n, k, fname):
    with open(fname, 'w') as f:
        f.write('%d %d\n' % (n, k))
        write_array(enc, f)

def write_sample_ky_encoding_gmp(enc, n, k, fname):
    write_sample_ky_encoding(enc, n, k, fname)

def write_sample_ky_matrix(P, k, l, fname):
    with open(fname, 'w') as f:
        f.write('%d %d\n' % (k, l))
        write_matrix(P, f)

def write_sample_ky_matrix_cached(k, l, h, T, fname):
    with open(fname, 'w') as f:
        f.write('%d %d\n' % (k, l))
        write_array(h, f)
        write_matrix(T, f)

def write_sample_fdr(n, fname):
    with open(fname, 'w') as f:
        f.write('%d\n' % (n,))

def write_sample_inversion_bernoulli(a, M, fname):
    with open(fname, 'w') as f:
        f.write("%d %d\n" % (a, M))

def write_sample_rejection_uniform(Ms, M, n, fname):
    with open(fname, 'w') as f:
        f.write('%d %d\n' % (n, M,))
        write_array(Ms, f)

def write_sample_rejection_hash_table(T, Z, k, fname):
    with open(fname, 'w') as f:
        f.write('%d %d\n' % (k, Z))
        write_array(T, f)

def write_sample_rejection_binary_search(cdf, Z, k, fname):
    with open(fname, 'w') as f:
        f.write('%d %d\n' % (k, Z))
        write_array(cdf, f)

def write_sample_interval(cdf, Z, k, fname):
    with open(fname, 'w') as f:
        f.write('%d %d\n' % (k, Z))
        write_array(cdf, f)

def write_sample_alias(n, qs, Ms, j, fname):
    with open(fname, 'w') as f:
        f.write('%d\n' % (n,))
        write_array(qs, f)
        write_array(Ms, f)
        write_array(j, f)

def write_sample_alias_integers(Z, T, entropy, fname):
    """
    Écrit les tables préparées pour alias.integers dans un fichier
    pour le pipeline, dans le format attendu :

    Ligne 1 : Z (plage des probabilités possibles)
    Ligne 2 : nombre d’objets suivi des objets
    Ligne 3 : entropie
    """

    with open(fname, 'w') as f:
        # Ligne 1 : plage des probabilités possibles
        f.write('%d\n' % Z)
        # print(f"[DEBUG] Ligne 1 écrite : {Z}")

        # Ligne 2 : nombre d’objets + objets de la distribution
        write_array(T, f)
        # print(f"[DEBUG] Ligne 2 écrite : {T}")

        # Ligne 3 : entropie
        f.write('%.5f\n' % entropy)
        #print(f"[DEBUG] Ligne 3 écrite : {entropy:.5f}")


def write_sample_alias_integers_old(Z, T, entropy, fname):
    write_sample_alias_integers(Z, T, entropy, fname)

def write_sample_alias_integers_gmp(Z, T, entropy, fname):
    write_sample_alias_integers(Z, T, entropy, fname)


def write_sample_alias_rust(Z, Ms, entropy, fname):
    """
    Écrit l'entrée brute attendue par l'implémentation C de alias.rust :
    poids entiers + entropie
    """
    with open(fname, 'w') as f:
        f.write('%d\n' % Z)
        write_array(Ms, f)
        f.write('%.5f\n' % entropy)

def write_sample_alias_rust_gmp(Z, Ms, entropy, fname):
    write_sample_alias_rust(Z, Ms, entropy, fname)

def write_sample_aldr(n, Ms, entropy, fname):
    """
    Écrit les poids pour ALDR dans un fichier
    Format identique à alias_integers :
    Ligne 1 : n (nombre d'éléments)
    Ligne 2 : Ms (poids entiers)
    Ligne 3 : entropie
    """
    with open(fname, 'w') as f:
        f.write('%d\n' % n)
        write_array(Ms, f)
        f.write('%.5f\n' % entropy)

def write_sample_aldr_gmp(n, Ms, entropy, fname):
    write_sample_aldr(n, Ms, entropy, fname)

def write_sample_alias_fractions(Z, T, entropy, fname):
    """
    Écrit les poids pour alias.fractions dans un fichier
    Format identique à alias_integers :
    Ligne 1 : n (nombre d'éléments)
    Ligne 2 : Ms (poids entiers)
    Ligne 3 : entropie
    """
    with open(fname, 'w') as f:
        f.write('%d\n' % Z)
        write_array(T, f)
        f.write('%.5f\n' % entropy)

def write_sample_alias_fractions_gmp(Z, T, entropy, fname):
    write_sample_alias_fractions(Z, T, entropy, fname)