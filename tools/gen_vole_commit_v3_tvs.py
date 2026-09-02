from textwrap import wrap
import sys
import importlib.util

def load_tvs(path):
    spec = importlib.util.spec_from_file_location("generated_data", path)
    if spec is None or spec.loader is None:
        raise ImportError(f"Could not load {path}")

    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)

    return module.TVS

def fmt_bytes(hx):
    data = bytes.fromhex(hx)
    return '\n'.join(wrap(', '.join(f'0x{x:02x}' for x in data), 100, initial_indent=' '*4, subsequent_indent=' '*4)) + '\n'

def fmt_tvs(param, data):
    struct = f'vole_commit_v3_tvs<faest::v3::{param}>'
    s = ''

    s += f'template <>\n'
    s += f'decltype({struct}::ell)\n'
    s += f'{struct}::ell = {data['ell']};\n'
    s += '\n'

    s += f'template <>\n'
    s += f'decltype({struct}::nmask)\n'
    s += f'{struct}::nmask = {data['nmask']};\n'
    s += '\n'

    s += f'template <>\n'
    s += f'decltype({struct}::crtmults)\n'
    s += f'{struct}::crtmults = {data['crtmults']};\n'
    s += '\n'

    s += f'template <>\n'
    s += f'decltype({struct}::seed)\n'
    s += f'{struct}::seed = {{\n'
    s += fmt_bytes(bytes(range(data['secpar'] // 8)).hex())
    s += f'}};\n'
    s += '\n'

    for tv in ['com', 'chall', 'hashed_u', 'hashed_c', 'hashed_mV', 'hashed_mQ', 'hashed_barU', 'hashed_barV', 'hashed_barQ', 'hashed_cmult']:
        s += f'template <>\n'
        s += f'decltype({struct}::{tv})\n'
        s += f'{struct}::{tv} = {{\n'
        s += fmt_bytes(data[tv])
        s += f'}};\n'
        s += '\n'
    return s

def fmt_all(TVS):
    print('#include "test_vole_commit_tvs_v3.hpp"')
    print()
    print('namespace test_vole_commit_tvs_v3 {')
    print()
    for name, data in TVS.items():
        print(fmt_tvs(name, data))
        print()
    print('} // namespace test_vole_commit_tvs_v3')

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <path>")
        sys.exit(1)
    path = sys.argv[1]
    TVS = load_tvs(path)
    fmt_all(TVS)
