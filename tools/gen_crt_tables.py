#!/usr/bin/env python3

import argparse
import json
import sys
from pathlib import Path
from math import ceil
from pprint import pprint

ENABLE_PUBLIC_ROW_LOOKUP = False

def bit_transpose(rows, cols, matrix):
    assert len(matrix) == rows
    assert all(x < 2**cols for x in matrix)
    new = [0] * cols
    for r in range(rows):
        for c in range(cols):
            b = (matrix[r] >> c) & 1
            new[c] |= b << r
    return new


def poly_div(p, q):
    assert q != 0
    t = 0
    deg_p = p.bit_length() - 1
    deg_q = q.bit_length() - 1
    while deg_p >= deg_q:
        deg_diff = deg_p - deg_q
        p ^= q << deg_diff
        t ^= 1 << deg_diff
        if p == 0:
            break
        deg_p = p.bit_length() - 1
    return t


def even_split(N, K):
    M = (N + K - 1) // K;
    S = N // M;
    R = N % M;
    return (S + 1, R, S, M - R);

def even_split_ks(N, K):
    k_max, n_max, k_min, n_min = even_split(N, K)
    return [k_max] * n_max + [k_min] * n_min


class TableGenerator:

    def __init__(self, data):
        self.data = data

    def get_crt_version(self):
        secpar = self.data['lambda']
        tau = self.data['tau']
        delta_bits = self.data['n_delta_bits']
        return f'{secpar}_{tau}_{delta_bits}'

    def make_header_file_header(self):
        s = '// clang-format off\n'
        s += '\n'
        s += '#pragma once\n'
        s += '\n'
        s += 'namespace faest {'
        return s

    def make_header_file_footer(self):
        return '} // namespace faest'

    def make_source_file_header(self):
        s = '// clang-format off\n'
        s += '\n'
        s += '#include "crt_constants.hpp"\n'
        s += '\n'
        s += 'namespace faest {'
        return s

    def make_source_file_footer(self):
        return '} // namespace faest'

    def gen_using(self):
        cver = self.get_crt_version()
        secpar = self.data['lambda']
        tau = self.data['tau']
        delta_bits = self.data['n_delta_bits']
        s = f'using CRT_CONSTANTS_{cver} = CRT_CONSTANTS<secpar::s{secpar}, {tau}, {delta_bits}>;'
        return s

    def gen_n_mult_assert(self):
        secpar = self.data['lambda']
        tau = self.data['tau']
        delta_bits = self.data['n_delta_bits']
        n_mult = self.data['n_gates']
        s = f'static_assert({n_mult} == compute_crt_num_mult(secpar::s{secpar}, {tau}, {delta_bits}), "Generated n_mult does not match constants.hpp.");'
        return s

    def gen_n_mult(self):
        cver = self.get_crt_version()
        secpar = self.data['lambda']
        tau = self.data['tau']
        delta_bits = self.data['n_delta_bits']
        n_mult = self.data['n_gates']
        s = ''
        s += f'template<>\n'
        s += f'struct N_MULT_TRAIT<secpar::s{secpar}, {tau}, {delta_bits}>\n'
        s += '{\n'
        s += f'    constexpr static std::size_t N_MULT = {n_mult};\n'
        s += '};\n'

        return s

    def gen_m_i_values(self):
        cver = self.get_crt_version()
        secpar = self.data['lambda']
        delta_bits = self.data['n_delta_bits']
        m_is = [int(x, 16) for x in self.data['tree_moduli_hex']]
        barret_vals = [poly_div(1 << 64, m_i) for m_i in m_is]
        #  assert m_tree.bit_length() == delta_bits + 1
        s = ''
        s += f'template <>\n'
        s += f'inline constexpr decltype(CRT_CONSTANTS_{cver}::M_i)\n'
        #  s += f'constexpr auto '
        s += f'CRT_CONSTANTS_{cver}::M_i = {{\n'
        for i in range(ceil(len(m_is) / 8)):
            s += ' ' * 4 + ', '.join(f'0x{x:04x}' for x in m_is[i*8:(i+1)*8]) + ',\n'
        s += '};'
        s += '\n'
        s += f'template <>\n'
        s += f'inline constexpr decltype(CRT_CONSTANTS_{cver}::DIV_X64_by_M_i)\n'
        #  s += f'constexpr auto '
        s += f'CRT_CONSTANTS_{cver}::DIV_X64_by_M_i = {{\n'
        for i in range(ceil(len(barret_vals) / 4)):
            s += ' ' * 4 + ', '.join(f'0x{x:016x}' for x in barret_vals[i*4:(i+1)*4]) + ',\n'
        s += '};'
        return s

    def gen_m_tree_decl(self):
        cver = self.get_crt_version()
        s = ''
        s += 'template <>\n'
        s += f'decltype(CRT_CONSTANTS_{cver}::M_TREE)\n'
        s += f'CRT_CONSTANTS_{cver}::M_TREE;\n'
        return s

    def gen_m_tree(self):
        cver = self.get_crt_version()
        secpar = self.data['lambda']
        delta_bits = self.data['n_delta_bits']
        m_tree = int(self.data['M_tree_hex'], 16)
        assert m_tree.bit_length() == delta_bits + 1
        s = ''
        s += f'template<>\n'
        s += f'alignas(CRT_CONSTANTS_{cver}::ALIGNMENT)\n'
        s += f'inline constexpr decltype(CRT_CONSTANTS_{cver}::M_TREE) CRT_CONSTANTS_{cver}::M_TREE = {{\n'
        s += ' ' * 4 + ', '.join(f'0x{x:02x}' for x in m_tree.to_bytes(secpar // 8, 'little')) + '\n'
        s += '};'
        return s

    def gen_lift_columns_index_table(self, name, rows, columns, lookup_bits, data):
        assert len(data) >= rows
        assert all(x == 0 for x in data[rows:])
        assert all(x < 2**columns for x in data)

        cver = self.get_crt_version()
        name = f'CRT_CONSTANTS_{cver}::{name}'
        s = ''
        s += f'template<>\n'
        s += f'decltype({name}) {name} = {{{{\n'
        offset = 0
        for k_i in even_split_ks(columns, lookup_bits):
        #  for i in range(ceil(columns / lookup_bits)):
            assert k_i <= lookup_bits
            bites = []
            for j in range(rows):
                #  bites.append((data[j] >> (i * lookup_bits)) & ((1 << lookup_bits) - 1))
                bites.append((data[j] >> offset) & ((1 << k_i) - 1))
            s += ' ' * 4 + '{'
            s += ', '.join(f'0x{b:02x}' for b in bites)
            s += '},\n'
            offset += k_i
        s += '}};\n'
        return s

    def gen_lift_public_row_lookup_table(self, name, rows, columns, lookup_bits, data):
        if not ENABLE_PUBLIC_ROW_LOOKUP:
            return ''
        assert len(data) >= rows
        assert all(x == 0 for x in data[rows:])
        assert all(x < 2**columns for x in data)

        data_trans = bit_transpose(rows, columns, data)
        assert bit_transpose(columns, rows, data_trans) == data
        cver = self.get_crt_version()
        name = f'CRT_CONSTANTS_{cver}::{name}'
        s = ''
        s += f'template<>\n'
        s += f'alignas(CRT_CONSTANTS_{cver}::ALIGNMENT) const\n'
        s += f'decltype({name}) {name} = {{{{\n'
        for i in range(ceil(columns / lookup_bits)):
            s += ' ' * 4 + '{{\n'
            chunk = data_trans[i * lookup_bits:(i+1) * lookup_bits]
            entries = [0] * (1 << len(chunk))
            for k in range(1 << len(chunk)):
                for bit_i in range(len(chunk)):
                    if k & (1 << bit_i):
                        entries[k] ^= chunk[bit_i]
            for e in entries:
                words = []
                for l in range(ceil(rows / 64)):
                    words.append(e & ((1 << 64) - 1))
                    e >>= 64
                s += ' ' * 8 + '{'
                s += ', '.join(f'0x{w:016x}' for w in words)
                s += '},\n'
            s += ' ' * 4 + '}},\n'
        s += '}};\n'
        return s

    def gen_lift_row_table(self, name, rows, columns, data):
        assert len(data) >= rows
        assert all(x == 0 for x in data[rows:])
        assert all(x < 2**columns for x in data)

        data_trans = bit_transpose(rows, columns, data)
        assert bit_transpose(columns, rows, data_trans) == data
        cver = self.get_crt_version()
        name = f'CRT_CONSTANTS_{cver}::{name}'
        s = ''
        s += f'template<>\n'
        s += f'decltype({name}) {name} = {{{{\n'
        entries = []
        for j in range(columns):
            entries.append(data_trans[j])
        for e in entries:
            words = []
            for l in range(ceil(rows / 64)):
                words.append(e & ((1 << 64) - 1))
                e >>= 64
            s += ' ' * 8 + '{'
            s += ', '.join(f'0x{w:016x}' for w in words)
            s += '},\n'
        s += '}};\n'
        s += '\n'
        return s

    def gen_w_crt_tables(self):
        secpar = self.data['lambda']
        delta_bits = self.data['n_delta_bits']
        tau = self.data['tau']
        w_crt = [int(h, 16) for h in self.data['W_crt']]
        assert len(w_crt) == secpar
        assert all(x == 0 for x in w_crt[delta_bits:])
        w_crt = w_crt[:delta_bits]
        assert all(x < 2**delta_bits for x in w_crt)

        lookup_bits = 4
        assert lookup_bits <= 8
        delta_lookup_bits = 4
        assert delta_lookup_bits <= 8

        s = ''
        s += self.gen_lift_columns_index_table('W_CRT_LIFT_COLUMNS_INDEX_TAB', delta_bits, delta_bits, lookup_bits, w_crt)
        s += '\n'
        s += self.gen_lift_row_table('W_CRT_LIFT_ROW_TAB', delta_bits, delta_bits, w_crt)
        s += '\n'
        s += self.gen_lift_public_row_lookup_table('W_CRT_LIFT_PUBLIC_ROW_LOOKUP_TAB', delta_bits, delta_bits, delta_lookup_bits, w_crt)
        s += '\n'
        return s;

    def gen_w_gate_tables(self):
        secpar = self.data['lambda']
        delta_bits = self.data['n_delta_bits']
        tau = self.data['tau']
        n_mult = self.data['n_gates']
        w_gate = [int(h, 16) for h in self.data['W_gate']]
        assert len(w_gate) == secpar
        assert all(x < 2**n_mult for x in w_gate)

        col_lookup_bits = 4
        assert col_lookup_bits <= 8
        row_lookup_bits = 4
        assert row_lookup_bits <= 8

        s = ''
        s += self.gen_lift_columns_index_table('W_GATE_LIFT_COLUMNS_INDEX_TAB', secpar, n_mult, col_lookup_bits, w_gate)
        s += '\n'
        s += self.gen_lift_row_table('W_GATE_LIFT_ROW_TAB', secpar, n_mult, w_gate)
        s += '\n'
        s += self.gen_lift_public_row_lookup_table('W_GATE_LIFT_PUBLIC_ROW_LOOKUP_TAB', secpar, n_mult, row_lookup_bits, w_gate)
        s += '\n'
        return s;

    def gen_table_decls(self):
        cver = self.get_crt_version()
        s = ''

        name = f'CRT_CONSTANTS_{cver}::W_CRT_LIFT_COLUMNS_INDEX_TAB'
        s += 'template <>\n'
        s += f'decltype({name}) {name};\n'

        name = f'CRT_CONSTANTS_{cver}::W_CRT_LIFT_ROW_TAB'
        s += 'template <>\n'
        s += f'decltype({name}) {name};\n'

        if ENABLE_PUBLIC_ROW_LOOKUP:
            name = f'CRT_CONSTANTS_{cver}::W_CRT_LIFT_PUBLIC_ROW_LOOKUP_TAB'
            s += 'template <>\n'
            s += f'decltype({name}) {name};\n'

        name = f'CRT_CONSTANTS_{cver}::W_TREE_LIFT_ROW_TAB'
        s += 'template <>\n'
        s += f'decltype({name}) {name};\n'

        if ENABLE_PUBLIC_ROW_LOOKUP:
            name = f'CRT_CONSTANTS_{cver}::W_TREE_LIFT_PUBLIC_ROW_LOOKUP_TAB'
            s += 'template <>\n'
            s += f'decltype({name}) {name};\n'

        name = f'CRT_CONSTANTS_{cver}::W_GATE_LIFT_COLUMNS_INDEX_TAB'
        s += 'template <>\n'
        s += f'decltype({name}) {name};\n'

        name = f'CRT_CONSTANTS_{cver}::W_GATE_LIFT_ROW_TAB'
        s += 'template <>\n'
        s += f'decltype({name}) {name};\n'

        if ENABLE_PUBLIC_ROW_LOOKUP:
            name = f'CRT_CONSTANTS_{cver}::W_GATE_LIFT_PUBLIC_ROW_LOOKUP_TAB'
            s += 'template <>\n'
            s += f'decltype({name}) {name};\n'

        name = f'CRT_CONSTANTS_{cver}::F_LIFT_ROW_TAB'
        s += 'template <>\n'
        s += f'decltype({name}) {name};\n'

        name = f'CRT_CONSTANTS_{cver}::G_LIFT_ROW_TAB'
        s += 'template <>\n'
        s += f'decltype({name}) {name};\n'

        name = f'CRT_CONSTANTS_{cver}::G_LIFT_COLUMNS_INDEX_TAB'
        s += 'template <>\n'
        s += f'decltype({name}) {name};\n'

        return s

    def gen_w_tree_tables(self):
        lookup_bits = 4
        assert lookup_bits <= 8

        secpar = self.data['lambda']
        delta_bits = self.data['n_delta_bits']
        tau = self.data['tau']
        w_tree = [int(h, 16) for h in self.data['W_tree']]
        assert len(w_tree) == secpar
        assert all(x < 2**delta_bits for x in w_tree)

        s = ''
        s += self.gen_lift_row_table('W_TREE_LIFT_ROW_TAB', secpar, delta_bits, w_tree)
        s += '\n'
        s += self.gen_lift_public_row_lookup_table('W_TREE_LIFT_PUBLIC_ROW_LOOKUP_TAB', secpar, delta_bits, lookup_bits, w_tree)
        s += '\n'

        return s

    def gen_f_tables(self):
        secpar = self.data['lambda']
        n_mult = self.data['n_gates']
        f = [int(h, 16) for h in self.data['F']]
        assert len(f) == n_mult
        assert all(x < 2**secpar for x in f)

        s = ''
        s += self.gen_lift_row_table('F_LIFT_ROW_TAB', n_mult, secpar, f)
        s += '\n'

        return s;

    def gen_g_tables(self):
        secpar = self.data['lambda']
        delta_bits = self.data['n_delta_bits']
        n_mult = self.data['n_gates']
        g = [int(h, 16) for h in self.data['G']]
        assert len(g) == n_mult
        assert all(x < 2**delta_bits for x in g)

        lookup_bits = 4
        assert lookup_bits <= 8

        s = ''
        s += self.gen_lift_row_table('G_LIFT_ROW_TAB', n_mult, delta_bits, g)
        s += '\n'
        s += self.gen_lift_columns_index_table('G_LIFT_COLUMNS_INDEX_TAB', n_mult, delta_bits, lookup_bits, g)
        s += '\n'

        return s;

    def print_header(self):
        print(self.make_header_file_header())
        print()
        print(self.gen_using())
        print()
        print(self.gen_n_mult())
        print()
        print(self.gen_m_i_values())
        print()
        print(self.gen_m_tree())
        print()
        print(self.gen_table_decls())
        print()
        print(self.make_header_file_footer())

    def print_source(self):
        print(self.make_source_file_header())
        print()
        print(self.gen_w_crt_tables())
        print()
        print(self.gen_w_tree_tables())
        print()
        print(self.gen_w_gate_tables())
        print()
        print(self.gen_f_tables())
        print()
        print(self.gen_g_tables())
        print()
        print(self.make_source_file_footer())


def parse_args():
    parser = argparse.ArgumentParser(
        description="Generate CRT tables for the optimized FAEST implementation."
    )
    parser.add_argument(
        "input",
        type=Path,
        help="Path to the input JSON file generated by the script in faest-spec",
    )
    parser.add_argument(
        "--header",
        action="store_true",
        help="Generate a header corresponding to source file",
    )
    return parser.parse_args()


def load_json(path: Path):
    try:
        with path.open("r", encoding="utf-8") as f:
            return json.load(f)
    except FileNotFoundError:
        sys.exit(f"Error: file not found: {path}")
    except json.JSONDecodeError as e:
        sys.exit(f"Error: invalid JSON ({e})")


def main():
    args = parse_args()
    data = load_json(args.input)
    tabgen = TableGenerator(data)
    if args.header:
        tabgen.print_header()
    else:
        tabgen.print_source()


if __name__ == "__main__":
    main()
