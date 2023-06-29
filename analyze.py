#!/usr/bin/env python3

import pandas as pd
import json
import sys

def load_data(path):
    data = []
    with open(path, 'r') as f:
        for l in f:
            data.append(json.loads(l))

    return pd.concat(pd.json_normalize(rd) for rd in data)


def analyze(df):
    columns_to_keep = [
        'variant',
        'sig_size_bytes',
        'sk_size_bytes',
        'pk_size_bytes',
        'keygen.mean_ms',
        'sign.mean_ms',
        'verify.mean_ms',
    ]

    df['keygen.mean_ms'] = df['keygen.mean_us'] / 1000
    df['sign.mean_ms'] = df['sign.mean_us'] / 1000
    df['verify.mean_ms'] = df['verify.mean_us'] / 1000

    for c in df.columns:
        if c not in columns_to_keep:
            del df[c]

    return df


def print_latex_table(impl, df, indentation=4):
    if impl == 'opt':
        label = 'tab:AVX2perf'
        caption = '''
        Benchmark results for the architecture specific implementation for x86-64 with AVX2.'
        '''.strip()
    elif impl == 'ref':
        label = 'tab:refperf'
        caption = '''
        Benchmark results for the reference implementation.'
        '''.strip()
    else:
        assert False
    variant_macros = {
        'FAEST_128S': r'\faestls',
        'FAEST_128F': r'\faestlf',
        'FAEST_192S': r'\faestms',
        'FAEST_192F': r'\faestmf',
        'FAEST_256S': r'\faesths',
        'FAEST_256F': r'\faesthf',
        'FAEST_EM_128S': r'\faestEMls',
        'FAEST_EM_128F': r'\faestEMlf',
        'FAEST_EM_192S': r'\faestEMms',
        'FAEST_EM_192F': r'\faestEMmf',
        'FAEST_EM_256S': r'\faestEMhs',
        'FAEST_EM_256F': r'\faestEMhf',
    }
    indent = ' ' * indentation

    print('%%% START GENERATED TABLE %%%')
    print(r'\begin{table}[tp]')
    print(1 * indent + r'\lennart{We will update the numbers in this table!}')
    print(1 * indent + r'\begin{center}')
    print(2 * indent + r'\begin{tabular}{')
    print(3 * indent + r'l')
    print(3 * indent + r'S[table-format=1.3, table-auto-round, group-minimum-digits=3]')
    print(3 * indent + r'S[table-format=3.3, table-auto-round, group-minimum-digits=3]')
    print(3 * indent + r'S[table-format=3.3, table-auto-round, group-minimum-digits=3]')
    print(3 * indent + r'S[table-format=2, table-auto-round, group-minimum-digits=3]')
    print(3 * indent + r'S[table-format=2, table-auto-round, group-minimum-digits=3]')
    print(3 * indent + r'S[table-format=5, table-auto-round, group-minimum-digits=3]')
    print(2 * indent + r'}')
    print(3 * indent + r'\toprule')
    print(3 * indent + r'\multicolumn{1}{c}{\multirow{2}{*}{Scheme}} & \multicolumn{3}{c}{Runtimes in \si{\milli\second}} & \multicolumn{3}{c}{Sizes in \si{\byte}} \\')
    print(3 * indent + r'\cmidrule(lr){2-4} \cmidrule(l){5-7}')
    print(3 * indent + r'    & {\(\keygen\)} & {\(\sign\)} & {\(\verify\)} & {\(\sk\)} & {\(\pk\)} & {Signature} \\')
    print(3 * indent + r'\midrule')
    for index, r in df.iterrows():
        if r['variant'] == 'FAEST_EM_128S':
            print(3 * indent + r'\midrule')

        line = 3 * indent
        line += f"\\({variant_macros[r['variant']]:10s}\\)"
        line += f" & {r['keygen.mean_ms']:10f}"
        line += f" & {r['sign.mean_ms']:16f}"
        line += f" & {r['verify.mean_ms']:16f}"
        line += f" & {r['sk_size_bytes']:2d}"
        line += f" & {r['pk_size_bytes']:2d}"
        line += f" & {r['sig_size_bytes']:5d}"
        line += r" \\"
        print(line)
    print(3 * indent + r'\bottomrule')
    print(2 * indent + r'\end{tabular}')
    print(1 * indent + r'\end{center}')
    print(1 * indent + r'\caption{' + caption + '}')
    print(1 * indent + r'\label{' + label + '}')
    print(r'\end{table}')
    print('%%% END GENERATED TABLE %%%')


def print_markdown_table(df, indentation=4):
    print('{% comment %}')
    print('   START GENERATED TABLE')
    print('{% endcomment %}')
    print(r'|---')
    print(r'| : Scheme : |  : Runtimes in milliseconds :  \|\||  : Sizes in bytes :  \|\|')
    print(r'| ^^         | : KeyGen : | : Sign : | : Verify : | : sk : | : pk : | : sig :')
    print(r'|:-|-:|-:|-:|-:|-:|-:')
    #print(r'\multicolumn{1}{c}{\multirow{2}{*}{Scheme}} & \multicolumn{3}{c}{Runtimes in \si{\milli\second}} & \multicolumn{3}{c}{Sizes in \si{\byte}} \\')
    #print(r'\cmidrule(lr){2-4} \cmidrule(l){5-7}')
    for index, r in df.iterrows():
        if r['variant'] == 'FAEST_EM_128S':
            print(r'|---')

        name = r['variant'].replace("_", "-")
        name = name[:-1] + name[-1].lower()

        line = "| "
        line += f"{name:16s}"
        line += f" | {r['keygen.mean_ms']:10f}"
        line += f" | {r['sign.mean_ms']:16f}"
        line += f" | {r['verify.mean_ms']:16f}"
        line += f" | {r['sk_size_bytes']:2d}"
        line += f" | {r['pk_size_bytes']:2d}"
        line += f" | {r['sig_size_bytes']:5d}"
        line += r""
        print(line)
    print(r'|---')
    print('{% comment %}')
    print('   END GENERATED TABLE')
    print('{% endcomment %}')


def main(argv):
    if len(argv) != 3 or argv[1] not in ['ref', 'opt', 'web']:
        print(f'usage: {sys.argv[0]} ref|opt|web <path>')
        exit(1)

    path = argv[2]
    opt = argv[1]

    data = load_data(path)
    data = analyze(data)
    #  print(data)
    if opt == 'web':
        print_markdown_table(data)
    else:
        print_latex_table(opt, data)


if __name__ == '__main__':
    main(sys.argv)
