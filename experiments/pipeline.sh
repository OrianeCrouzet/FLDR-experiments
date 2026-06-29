#!/bin/bash

set -e

# Set PYTHONPATH to find the discrete_sampling module
export PYTHONPATH="$(cd "$(dirname "$0")/.." && pwd)/build/lib:$PYTHONPATH"

if [ -z "${JOBS:-}" ]; then
  NCPU=$(python3 -c 'import multiprocessing; print(multiprocessing.cpu_count())')
else
  NCPU=${JOBS}
fi

# Detect GNU xargs (common on macOS via homebrew) or fallback to xargs
XARGS_BIN="xargs"
if command -v gxargs >/dev/null 2>&1; then
  XARGS_BIN="gxargs"
fi

set -u

stamp=${1}
cmd=${2}

# rej.enc == fldr
#SAMPLERS='alias.exact rej.enc  rej.table rej.uniform alias.integers aldr alias.rust alias.fractions'
#SAMPLERS='alias.rust alias.rust alias.integers_old alias.integers_old aldr aldr alias.exact alias.exact rej.enc rej.enc alias.fractions alias.fractions'
SAMPLERS='alias.integers_old'

#if [ ${cmd} = 'initialize' ]; then
 # N=$(echo ${stamp} | cut -d. -f2)
  #Z=$(echo ${stamp} | cut -d. -f3)
  #seed=$(echo ${stamp} | cut -d. -f4)
  #qs=$(echo ${SAMPLERS} | xargs -I% echo "'%'")
  #./dists.py generate-distributions N=${N} Z=${Z} seed=${seed} samplers="${qs}" thin=5;
  #exit 0;
#fi

if [ "${cmd}" = 'initialize' ]; then
  if [[ "${stamp}" =~ ^dists\.([0-9]+)\.([0-9]+)\.([0-9]+)$ ]]; then
    N=${BASH_REMATCH[1]}
    Z=${BASH_REMATCH[2]}
    seed=${BASH_REMATCH[3]}
    target_samplers="${SAMPLERS}"
  else
    # support des appels direct : ./pipeline.sh alias.integers initialize
    case "${stamp}" in
      interval|alias.exact|ky.enc|rej.binary|rej.enc|rej.matc|rej.table|rej.uniform|alias.integers|alias.integers_old|aldr|alias.rust|alias.fractions)
        N=5; Z=10; seed=1; target_samplers="${stamp}";
        ;;
      *)
        echo "Unknown sampler init key: ${stamp}" >&2;
        exit 2
        ;;
    esac
  fi

  echo "=== Initialize: Generating distributions ==="
  echo "N = ${N}, Z = ${Z}, seed = ${seed}, samplers = ${target_samplers}"

  for sampler in ${target_samplers}; do
      echo "--- Generating distributions for sampler: ${sampler} ---"
      ./dists.py generate-distributions N=${N} Z=${Z} seed=${seed} samplers="${sampler}" thin=5
      echo "Finished sampler: ${sampler}"
  done

  echo "=== All samplers initialized ==="
  exit 0
fi

# if [ ${cmd} = 'initialize-2' ]; then
# # Pour la future figure avec n qui varie et entropie fixée

#   # format attendu :
#   #   dists.H.maxpow.seed
#   # exemple :
#   #   dists.3.12.1
#   # => H=3 maxpow=12 seed=1

#   if [[ "${stamp}" =~ ^dists\.entropy\.([0-9]+([.][0-9]+)?)\.([0-9]+)\.([0-9]+)$ ]]; then
#     H=${BASH_REMATCH[1]}
#     maxpow=${BASH_REMATCH[3]}
#     seed=${BASH_REMATCH[4]}
#     target_samplers="${SAMPLERS}"

#   else
#     # appel direct :
#     # ./pipeline.sh alias.integers initialize
#     case "${stamp}" in
#       interval|alias.exact|ky.enc|rej.binary|rej.enc|rej.matc|rej.table|rej.uniform|alias.integers|aldr|alias.rust|alias.fractions)
#         H=10
#         maxpow=12
#         Z=-1
#         seed=1
#         target_samplers="${stamp}"
#         ;;
#       *)
#         echo "Unknown sampler init key: ${stamp}" >&2
#         exit 2
#         ;;
#     esac
#   fi

#   echo "=== Initialize: Generating distributions ==="
#   echo "H = ${H}, maxpow = ${maxpow}, seed = ${seed}, samplers = ${target_samplers}"

#   for sampler in ${target_samplers}; do
#       echo "--- Generating distributions for sampler: ${sampler} ---"

#       ./dists.py generate-distributions-entropy \
#           H=${H} \
#           maxpow=${maxpow} \
#           Z=-1 \
#           seed=${seed} \
#           samplers="${sampler}" \
#           thin=5

#       echo "Finished sampler: ${sampler}"
#   done

#   echo "=== All samplers initialized ==="
#   exit 0
# fi

if [ "${cmd}" = 'aggregate-sizes' ]; then
  for sampler in ${SAMPLERS}; do
      fn=${stamp}/${sampler}.sizes;
      # Portable file size: -f %z for BSD/macOS, -c %s for GNU/Linux
      stat -f %z ${stamp}/*.${sampler} 2>/dev/null > "${fn}" || stat -c %s ${stamp}/*.${sampler} > "${fn}";
      echo ${stamp}/*.${sampler};
      echo ${fn};
  done
  exit 0;
fi

# Pour charger nos *.dist dans un dossier, au lieu de les générer à la volée (mesures habituelles)
# Les distributions doivent avoir le même format que les distributions générées à la base !
# Commande : ./pipeline.sh <dir> measure-runtimes-existing <steps> [seed]
# steps = nombre de tirage par distribution et par sampler
# seed = graine aléatoire (optionnel, défaut 1)
# On effectue une copie du dossier des distributions pour les garder intacts,
# le nouveau dossier s'appelle <dir>-work et c'est dans ce dossier que les fichiers de runtime seront créés.
if [ "${cmd}" = 'measure-runtimes-existing' ]; then
  steps=${3}
  seed=${4:-1}
  if [ -z "${steps}" ]; then
      echo "Usage: $0 <dir> measure-runtimes-existing <steps> [seed]" >&2
      exit 1
  fi

  if [ ! -d "${stamp}" ]; then
      echo "Directory not found: ${stamp}" >&2
      exit 1
  fi

  work_dir="measures/$(basename "${stamp}")-work"
  mkdir -p "$(dirname "${work_dir}")"
  rm -rf "${work_dir}"
  cp -R "${stamp}" "${work_dir}"
  echo "=== Working copy created at ${work_dir} ==="
  stamp="${work_dir}"

  if ls "${stamp}"/*.dist >/dev/null 2>&1; then
      echo "=== Found .dist files in ${stamp}; generating sampler structures ==="
      python3 <<PY
from pathlib import Path
from fractions import Fraction
from discrete_sampling.construct import (
    construct_sample_alias,
    construct_sample_alias_integers,
    construct_sample_alias_rust,
    construct_sample_aldr,
    construct_sample_alias_fractions,
    construct_sample_interval,
    construct_sample_ky_encoding,
    construct_sample_ky_matrix,
    construct_sample_ky_matrix_cached,
    construct_sample_ky_approx_encoding,
    construct_sample_ky_approx_matrix,
    construct_sample_ky_approx_matrix_cached,
    construct_sample_rejection_uniform,
    construct_sample_rejection_hash_table,
    construct_sample_rejection_binary_search,
    construct_sample_rejection_encoding,
    construct_sample_rejection_matrix,
    construct_sample_rejection_matrix_cached,
)
from discrete_sampling.writeio import (
    write_sample_alias,
    write_sample_alias_integers,
    write_sample_alias_rust,
    write_sample_aldr,
    write_sample_alias_fractions,
    write_sample_interval,
    write_sample_ky_encoding,
    write_sample_ky_matrix,
    write_sample_ky_matrix_cached,
    write_sample_rejection_uniform,
    write_sample_rejection_hash_table,
    write_sample_rejection_binary_search,
)

stamp = Path('${stamp}')
structures = {
    'ky.enc': (construct_sample_ky_encoding, write_sample_ky_encoding),
    'ky.mat': (construct_sample_ky_matrix, write_sample_ky_matrix),
    'ky.matc': (construct_sample_ky_matrix_cached, write_sample_ky_matrix_cached),
    'ky.approx.enc': (construct_sample_ky_approx_encoding, write_sample_ky_encoding),
    'ky.approx.mat': (construct_sample_ky_approx_matrix, write_sample_ky_matrix),
    'ky.approx.matc': (construct_sample_ky_approx_matrix_cached, write_sample_ky_matrix_cached),
    'rej.uniform': (construct_sample_rejection_uniform, write_sample_rejection_uniform),
    'rej.table': (construct_sample_rejection_hash_table, write_sample_rejection_hash_table),
    'rej.binary': (construct_sample_rejection_binary_search, write_sample_rejection_binary_search),
    'rej.enc': (construct_sample_rejection_encoding, write_sample_ky_encoding),
    'rej.mat': (construct_sample_rejection_matrix, write_sample_ky_matrix),
    'rej.matc': (construct_sample_rejection_matrix_cached, write_sample_ky_matrix_cached),
    'interval': (construct_sample_interval, write_sample_interval),
    'alias.exact': (construct_sample_alias, write_sample_alias),
    'alias.integers': (construct_sample_alias_integers, write_sample_alias_integers),
    'alias.rust': (construct_sample_alias_rust, write_sample_alias_rust),
    'aldr': (construct_sample_aldr, write_sample_aldr),
    'alias.fractions': (construct_sample_alias_fractions, write_sample_alias_fractions),
}

for dist_path in sorted(stamp.glob('*.dist')):
    with dist_path.open() as f:
        Z = int(f.readline().strip())
        parts = f.readline().strip().split()
        n = int(parts[0])
        Ms = [int(x) for x in parts[1:]]
        entropy = float(f.readline().strip())
    p_target = [Fraction(m, Z) for m in Ms]

    for sampler in "${SAMPLERS}".split():
        if sampler not in structures:
            continue
        f_construct, f_write = structures[sampler]
        out_path = dist_path.with_name(dist_path.stem + '.' + sampler)
        struc = f_construct(p_target)
        if sampler in {'alias.integers', 'alias.rust'}:
            f_write(*struc, Z, entropy, str(out_path))
        else:
            f_write(*struc, entropy, str(out_path))
PY
  fi

  for sampler in ${SAMPLERS}; do
      echo "=== Measuring existing sampler: ${sampler} ==="
      fnames=$(ls "${stamp}"/*.${sampler} 2>/dev/null || true)
      if [ -z "${fnames}" ]; then
          echo "No files for sampler ${sampler} in ${stamp}" >&2
          continue
      fi
      tmp_jobs=$(mktemp)
      for f in ${fnames}; do
          u=${f}.runtime
          echo "./main.out.opt ${seed} ${steps} ${sampler} ${f} > ${u} && echo ${u}" >> "${tmp_jobs}"
      done
      cat "${tmp_jobs}" | ${XARGS_BIN} -P ${NCPU} -n1 -d'\n' -I% sh -c '%'
      wait
      rm -f "${tmp_jobs}"
  done

  # for sampler in ${SAMPLERS}; do
  #     echo "=== Measuring existing sampler: ${sampler} ==="
  #     rm -rf /tmp/w

  #     echo "[DEBUG] Listing files for sampler ${sampler} in ${stamp}"
  #     fnames=$(ls "${stamp}"/*.${sampler} 2>/dev/null || true)

  #     if [ -z "${fnames}" ]; then
  #         echo "[WARN] No files for sampler ${sampler} in ${stamp}" >&2
  #         continue
  #     fi

  #     for f in ${fnames}; do
  #         u=${f}.runtime
  #         echo "[DEBUG] Preparing job for file: ${f}"

  #         echo "echo '[START] sampler=${sampler} file=${f}' && \
  # ./main.out.opt ${seed} ${steps} ${sampler} ${f} > ${u} 2>&1 && \
  # echo '[DONE] ${u}' || echo '[ERROR] sampler=${sampler} file=${f}'" >> /tmp/w
  #     done

  #     echo "[DEBUG] Jobs to execute:"
  #     cat /tmp/w

  #     echo "[DEBUG] Launching gxargs with ${NCPU} workers"
  #     cat /tmp/w | gxargs -P ${NCPU} -n1 -d'\n' -I% sh -c '%'

  #     echo "[DEBUG] Waiting for all jobs to finish..."
  #     wait
  #     echo "[DEBUG] Done with sampler ${sampler}"
  # done

  # Reuse existing aggregation commands for readability and maintainability.
  echo "=== Aggregating sizes ==="
  "$0" ${stamp} aggregate-sizes

  echo "=== Aggregating runtimes ==="
  "$0" ${stamp} aggregate-runtimes ${steps}

  exit 0;
fi

if [ ${cmd} = 'measure-runtimes' ]; then
  steps=${3}
  seed=$(echo ${stamp} | cut -d. -f4)
  
  # Actual measurement phase
  echo "=== MEASUREMENT PHASE ==="
  for sampler in ${SAMPLERS}; do
      echo "=== Measuring sampler: ${sampler} ==="   # <-- log
      fnames=$(ls ${stamp}/*.${sampler});
      tmp_jobs=$(mktemp)
      for f in ${fnames}; do
          u=${f}.runtime
          echo "./main.out.opt ${seed} ${steps} ${sampler} ${f} > ${u} && echo ${u}" >> "${tmp_jobs}"
      done
      echo "Starting ${sampler} runs..."
      cat "${tmp_jobs}" | ${XARGS_BIN} -P ${NCPU} -n1 -d'\n' -I% sh -c '%'
      wait
      rm -f "${tmp_jobs}"
      echo "Finished ${sampler}"  # <-- log
  done
  exit 0;
fi

if [ ${cmd} = 'aggregate-runtimes' ]; then
  steps=${3}
  rm -f ${stamp}/avg_bits

  for sampler in ${SAMPLERS}; do
      echo "=== Aggregate runtimes : ${sampler} ==="
      fn_runtime=${stamp}/${sampler}.runtimes
      fn_calls=${stamp}/${sampler}.calls
      fn_bits=${stamp}/${sampler}.bits

      fnames=$(ls ${stamp}/*.${sampler}.runtime 2>/dev/null || true)

      rm -f ${fn_runtime} ${fn_calls} ${fn_bits}

      for f in ${fnames}; do
          echo $f
          cat ${f} | tail -n1 | cut -f2 -d ' ' >> ${fn_runtime}
          cat ${f} | tail -n1 | cut -f3 -d ' ' >> ${fn_calls}

          # calcul des bits ici
          # on divise par steps pour obtenir le nombre de bits par tirage
          cat ${f} | tail -n1 | awk -v steps=${steps} '{print ($3 * 32) / steps}' >> ${fn_bits}
      done

      echo ${fn_runtime}
      echo ${fn_calls}
      echo ${fn_bits}

      # moyenne pour ce sampler
      if [ -f ${fn_bits} ]; then
          avg_bits=$(awk '{s+=$1} END {print s/NR}' ${fn_bits})
          echo "${sampler} ${avg_bits}" >> ${stamp}/avg_bits
      fi

      echo "Finished ${sampler}"
  done


  echo "Averages written to ${stamp}/avg_bits"
  exit 0
fi

if [ ${cmd} = 'run-all-memory-runtime' ]; then
  Z=${3}
  Ns="${4}"
  for n in ${Ns}; do
      stamp=dists.${n}.${Z}.2
      echo ${stamp}
      ./pipeline.sh ${stamp} initialize;
      ./pipeline.sh ${stamp} aggregate-sizes;
      ./pipeline.sh ${stamp} measure-runtimes 1000000;
      ./pipeline.sh ${stamp} aggregate-runtimes 1000000;
  done
  exit 0
fi

# if [ ${cmd} = 'run-all-memory-runtime-2' ]; then
# # Pour la future figure avec n qui varie et entropie fixée
#   H=${3}
#   seed=${5:-2}

#   for maxpow in ${4}; do
#       stamp=dists.entropy.${H}.${maxpow}.${seed}
#       echo ${stamp}

#       ./pipeline.sh ${stamp} initialize-2;
#       ./pipeline.sh ${stamp} aggregate-sizes;
#       ./pipeline.sh ${stamp} measure-runtimes 1000000;
#       ./pipeline.sh ${stamp} aggregate-runtimes 1000000;
#   done
#   exit 0
# fi

# The Ns are designed to be linearly spaced on log scale.
Ns_pp='2 3 4 5 6 7 8 9 10 11 12 13 14 15 17 18 19 21 23 25 27 29 31 34 36 39 42 46 50 54 58 63 68 73 79 85 92 100 107 116 125 135 146 158 171 184 199 215 232 251 271 292 316 341 368 398 429 464 501 541 584 630 681 735 794 857 926 1000 1079 1165 1258 1359 1467 1584 1711 1847 1995 2154 2326 2511 2712 2928 3162 3414 3686 3981 4298 4641 5011 5411 5843 6309 6812 7356 7943 8576 9261 10000 12589 15848 19952 25118 31622 39810 50118 63095 79432'
Zs_pp='10 100 1000 10000 100000 1000000'
# Ns_pp='10 20 30'
# Zs_pp='10 15 50'
if [ ${cmd} = 'preprocess-initialize' ]; then
  seed=$(echo ${stamp} | cut -d. -f3);
  mkdir -p ${stamp};
  counter=0
  for n in ${Ns_pp}; do (
    for Z in ${Zs_pp}; do
      if [ ${n} -lt ${Z} ]; then
        d=dists.${n}.${Z}.${seed}
        ./dists.py generate-distributions \
            N=${n} Z=${Z} seed=${seed} samplers='none'\
            thin=600 offset=500;
        for x in $(ls ${d}/*); do
          fx=$(echo $x | tr '/' '.' | sed 's/d.00000.//g')
          dx=${stamp}/${fx};
          mv $x ${dx};
          echo ${dx};
        done
        rm -rf ${d};
      fi
    done
    wait;
  ) &
  counter=$((counter + 1))
  if [ ${counter} -eq 60 ]; then
    wait
    counter=0;
  fi
  done
  exit 0
fi

if [ ${cmd} = 'preprocess-measure' ]; then
  fnames=$(ls ${stamp}/*.dist);
  
  rm -rf ${stamp}/preprocess
  tmp_pre=$(mktemp)
  for fn in ${fnames}; do
      u=${fn%.dist}.preprocess
      echo "./preprocess.out.opt ${fn} > ${u}.c && echo ${u}.c" >> "${tmp_pre}"
  done
  cat "${tmp_pre}" | ${XARGS_BIN} -P ${NCPU} -n1 -d'\n' -I% sh -c '%'
  rm -f "${tmp_pre}"
  cat ${stamp}/*.preprocess.c > ${stamp}/preprocess
  echo ${stamp}/preprocess
  exit 0;
fi

if [ ${cmd} = 'preprocess-aggregate' ]; then
  seed=$(echo ${stamp} | cut -d. -f3);
  Zs=$(ls ${stamp}/dists.*.c | cut -f2 -d/ | cut -f3 -d. | sort | uniq);
  Ns=$(ls ${stamp}/dists.*.c | cut -f2 -d/ | cut -f2 -d. | sort -h | uniq);
  for Z in ${Zs}; do
    fout=${stamp}/aggregate.${Z}.preprocess
    rm -rf ${fout}.c;
    rm -rf ${fout}.cpp;
    for n in ${Ns}; do
      if [ ${n} -lt ${Z} ]; then
        cat ${stamp}/dists.${n}.${Z}.${seed}.preprocess.c >> ${fout}.c;
      fi
    done
    echo ${fout}.c
  done
  exit 0
fi

echo 'Unknown command' ${cmd};
exit 1
