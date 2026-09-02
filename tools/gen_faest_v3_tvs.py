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
    param = param[:-1] + '_' + param[-1]
    struct = f'faest_tvs<faest::v3::{param}>'
    s = ''

    #  s += f'template <>\n'
    #  s += f'decltype({struct}::seed)\n'
    #  s += f'{struct}::seed = {{\n'
    #  s += fmt_bytes(bytes(range(data['secpar'] // 8)).hex())
    #  s += f'}};\n'
    #  s += '\n'

    for name, key in [('sk', 'sk_packed'), ('pk', 'pk_packed'), ('witness', 'w'), ('signature', 'sig')]:
        s += f'template <>\n'
        s += f'decltype({struct}::{name})\n'
        s += f'{struct}::{name} = {{\n'
        s += fmt_bytes(data[key])
        s += f'}};\n'
        s += '\n'
    return s

def fmt_all(TVS):
    print('#include "parameters.hpp"')
    print('#include "test_faest_tvs.hpp"')
    print()
    print('// clang-format off')
    print()
    for name, data in TVS.items():
        print(fmt_tvs(name, data))
    print('// clang-format on')

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <path>")
        sys.exit(1)
    path = sys.argv[1]
    TVS = load_tvs(path)
    fmt_all(TVS)
