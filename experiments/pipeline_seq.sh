#!/bin/bash

# Version séquentielle de pipeline.sh pour les mesures de temps de la figure 4.
#
# pipeline.sh lance les mesures avec xargs -P (une par cœur) : les mesures se
# disputent les cœurs, le cache et la bande passante mémoire, et les threads
# producteurs de bits aléatoires s'ajoutent aux threads de tirage. Les temps
# mesurés sont alors gonflés (voir rapport_optimisation/).
#
# Ici, une seule mesure tourne à la fois. Les fichiers produits sont les mêmes
# que ceux de pipeline.sh (d.XXXXX.<sampler>.runtime, <sampler>.runtimes,
# .calls, .bits, .rejet, .sizes, avg_bits), donc le notebook
# figure-4-interpolation_Antoine.ipynb s'utilise sans modification.
#
# Différences avec pipeline.sh :
#   - aucune exécution en parallèle ;
#   - seulement les cinq versions _opt, plus alias.integers_v2 ;
#   - chaque sampler est mesuré une fois (pipeline.sh le mesurait deux fois et
#     gardait la seconde mesure) ;
#   - l'ordre est entrelacé : pour chaque distribution, tous les samplers à la
#     suite. Une dérive de la machine pendant l'exécution (température,
#     fréquence, autre programme) touche alors tous les samplers de la même façon.
#
# Commandes :
#   ./pipeline_seq.sh <dir> initialize <N> <Z> <seed>   (= pipeline.sh, génération)
#   ./pipeline_seq.sh <dir> measure-runtimes <steps>
#   ./pipeline_seq.sh <dir> measure-structure-sizes
#   ./pipeline_seq.sh <dir> aggregate-runtimes <steps>
#   ./pipeline_seq.sh <dir> run-all <steps>   (mesures + tailles + agrégation,
#                                               sur les distributions existantes)
#
# Exemple (figure 4, distributions déjà présentes) :
#   caffeinate -i ./pipeline_seq.sh distributions_entropy run-all 10000000

set -e
set -u

cd "$(dirname "$0")"

stamp=${1}
cmd=${2}

# Seulement les versions optimisées, et alias.integers_v2. Les fichiers des versions
# _gmp déjà présents dans <dir> ne sont ni recalculés ni effacés.
SAMPLERS='alias.integers_opt alias.integers_v2 rej.enc_opt aldr_opt alias.rust_opt alias.fractions_opt'

if [ "${cmd}" = 'initialize' ]; then
  # La génération des distributions ne mesure aucun temps : on garde celle de pipeline.sh.
  exec ./pipeline.sh "$@"
fi

if [ "${cmd}" = 'measure-runtimes' ]; then
  steps=${3}
  # Même graine que pipeline.sh (elle n'influence pas les tirages, qui utilisent arc4random).
  seed=$(echo ${stamp} | cut -d. -f4)

  dists=$(ls ${stamp}/*.dist)
  n_dists=$(echo "${dists}" | wc -l | tr -d ' ')
  echo "=== MEASUREMENT PHASE (séquentielle) : ${n_dists} distributions, ${steps} tirages ==="
  start=$(date +%s)
  k=0
  for d in ${dists}; do
      k=$((k + 1))
      base=${d%.dist}
      for sampler in ${SAMPLERS}; do
          f=${base}.${sampler}
          # alias.integers_v2 lit le même fichier que alias.integers_gmp (créé s'il manque).
          if [ ! -f "${f}" ] && [ "${sampler}" = 'alias.integers_v2' ] && [ -f "${base}.alias.integers_gmp" ]; then
              cp "${base}.alias.integers_gmp" "${f}"
          fi
          if [ ! -f "${f}" ]; then
              echo "Missing ${f}" >&2
              continue
          fi
          ./main.out.opt ${seed} ${steps} ${sampler} ${f} > ${f}.runtime
      done
      now=$(date +%s)
      elapsed=$((now - start))
      remaining=$((elapsed * (n_dists - k) / k))
      printf '[%d/%d] %s  (écoulé %dm%02ds, reste ~%dm%02ds)\n' \
          ${k} ${n_dists} "$(basename ${base})" \
          $((elapsed / 60)) $((elapsed % 60)) $((remaining / 60)) $((remaining % 60))
  done
  echo "=== Finished measurements ==="
  exit 0
fi

if [ "${cmd}" = 'measure-structure-sizes' ]; then
  for sampler in ${SAMPLERS}; do
      echo "=== Measuring structure sizes: ${sampler} ==="
      sizes_file="${stamp}/${sampler}.sizes"
      rm -f "${sizes_file}"
      for f in $(ls "${stamp}"/*.${sampler}); do
          size_out=$(mktemp)
          ../c/measure_sizes.out.opt "${sampler}" "${f}" "${size_out}"
          cat "${size_out}" >> "${sizes_file}"
          rm -f "${size_out}"
      done
      echo "Wrote ${sizes_file}"
  done
  exit 0
fi

if [ "${cmd}" = 'aggregate-runtimes' ]; then
  steps=${3}
  rm -f ${stamp}/avg_bits
  for sampler in ${SAMPLERS}; do
      fn_runtime=${stamp}/${sampler}.runtimes
      fn_calls=${stamp}/${sampler}.calls
      fn_bits=${stamp}/${sampler}.bits
      fn_rejet=${stamp}/${sampler}.rejet
      rm -f ${fn_runtime} ${fn_calls} ${fn_bits} ${fn_rejet}

      for f in $(ls ${stamp}/*.${sampler}.runtime); do
          tail -n1 ${f} | cut -f2 -d ' ' >> ${fn_runtime}
          tail -n1 ${f} | cut -f3 -d ' ' >> ${fn_calls}
          tail -n1 ${f} | cut -f4 -d ' ' >> ${fn_rejet}
          # bits par tirage = mots de 32 bits * 32 / steps
          tail -n1 ${f} | awk -v steps=${steps} '{print ($3 * 32) / steps}' >> ${fn_bits}
      done

      avg_bits=$(awk '{s+=$1} END {print s/NR}' ${fn_bits})
      echo "${sampler} ${avg_bits}" >> ${stamp}/avg_bits
      echo "=== Aggregated ${sampler} ==="
  done
  echo "Averages written to ${stamp}/avg_bits"
  exit 0
fi

if [ "${cmd}" = 'run-all' ]; then
  steps=${3:-10000000}
  ./pipeline_seq.sh "${stamp}" measure-runtimes "${steps}"
  ./pipeline_seq.sh "${stamp}" measure-structure-sizes
  ./pipeline_seq.sh "${stamp}" aggregate-runtimes "${steps}"
  exit 0
fi

echo 'Unknown command' ${cmd}
exit 1
