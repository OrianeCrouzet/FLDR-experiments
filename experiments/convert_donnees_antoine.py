#!/usr/bin/env python3
from pathlib import Path
from math import log2
from decimal import Decimal, localcontext
import argparse
import hashlib


def parse_weights(path, abs_weights=False):
    with open(path, 'r', encoding='utf-8') as f:
        weights = []
        for lineno, line in enumerate(f, start=1):
            line = line.strip()
            if not line:
                continue
            try:
                value = int(line)
            except ValueError as exc:
                raise ValueError(f'Invalid integer on line {lineno} of {path}: {line}') from exc
            if value < 0:
                if abs_weights:
                    value = abs(value)
                else:
                    raise ValueError(f'Negative weight on line {lineno} of {path}: {value}')
            weights.append(value)
    if not weights:
        raise ValueError(f'No weights found in {path}')
    return weights


def compute_entropy(weights):
    Z = sum(weights)
    if Z == 0:
        return 0.0

    with localcontext() as ctx:
        ctx.prec = max(80, len(str(Z)) * 2)
        Z_dec = Decimal(Z)
        log2_dec = Decimal(2).ln()
        entropy = Decimal(0)
        for w in weights:
            if w == 0:
                continue
            p = Decimal(w) / Z_dec
            entropy -= p * (p.ln() / log2_dec)

    return float(entropy)


def write_dist_file(destination: Path, weights):
    n = len(weights)
    Z = sum(weights)
    entropy = compute_entropy(weights)
    destination.parent.mkdir(parents=True, exist_ok=True)
    with destination.open('w', encoding='utf-8') as f:
        f.write(f'{Z}\n')
        f.write(f'{n} ' + ' '.join(str(w) for w in weights) + '\n')
        f.write(f'{entropy:.5f}\n')


def make_output_folder_name(dist_type, n, Z, seed):
    folder_name = f'dists.custom.{dist_type}.{n}.{Z}.{seed}'
    if len(folder_name) <= 120:
        return folder_name
    # Fallback for huge total weights so path names stay within OS limits.
    z_hash = hashlib.sha1(str(Z).encode('utf-8')).hexdigest()[:16]
    return f'dists.custom.{dist_type}.{n}.hash{z_hash}.{seed}'


def main():
    parser = argparse.ArgumentParser(
        description='Convert donnees_*.txt files into .dist distributions for the experiments pipeline.'
    )
    parser.add_argument(
        '--source', '-s',
        default='donnees_antoine',
        help='Source directory containing donnees_*.txt files (default: donnees_antoine)',
    )
    parser.add_argument(
        '--dist-type', '-t',
        default='very-biased',
        help='Distribution type label used in the output folder name',
    )
    parser.add_argument(
        '--seed', '-S',
        type=int,
        default=2,
        help='Seed value used in the output folder name (default: 2)',
    )
    parser.add_argument(
        '--force', '-f',
        action='store_true',
        help='Overwrite existing .dist files if present',
    )
    parser.add_argument(
        '--abs-weights',
        action='store_true',
        help='Treat negative weights as their absolute values instead of failing.',
    )
    args = parser.parse_args()

    base_dir = Path(__file__).resolve().parent
    src_dir = base_dir / args.source

    if not src_dir.is_dir():
        raise SystemExit(f'Source directory not found: {src_dir}')

    txt_files = sorted(src_dir.glob('donnees_*.txt'))
    if not txt_files:
        raise SystemExit(f'No donnees_*.txt files found in {src_dir}')

    grouped = {}
    for txt_path in txt_files:
        weights = parse_weights(txt_path, abs_weights=args.abs_weights)
        n = len(weights)
        Z = sum(weights)
        grouped.setdefault((n, Z), []).append((txt_path, weights))

    print(f'Converting {len(txt_files)} files from {src_dir}')
    for (n, Z), items in sorted(grouped.items()):
        folder_name = make_output_folder_name(args.dist_type, n, Z, args.seed)
        out_dir = base_dir / folder_name
        out_dir.mkdir(parents=True, exist_ok=True)
        print(f'  Writing {len(items)} distribution(s) to {out_dir}')

        for idx, (txt_path, weights) in enumerate(items):
            dist_name = f'd.{idx:05d}.dist'
            dist_path = out_dir / dist_name
            if dist_path.exists() and not args.force:
                raise SystemExit(f'Output file already exists: {dist_path} (use --force to overwrite)')
            write_dist_file(dist_path, weights)
            print(f'    {txt_path.name} -> {dist_name} (n={n}, Z={Z})')

    print('Done.')


if __name__ == '__main__':
    main()
