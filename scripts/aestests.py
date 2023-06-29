from Crypto.Cipher import AES
from py3rijndael import Rijndael
from struct import pack
from collections import defaultdict

import random

random.seed(42)

num_ctr_blocks = 15
num_keys = 3
params = [
    (16, 16), (24, 16), (32, 16), (24, 24), (32, 32),
]

def num_rounds(ks):
    if ks == 16:
        return 11
    elif ks == 24:
        return 13
    elif ks == 32:
        return 15
    else:
        assert False

def xor(xs, ys):
    assert len(xs) == len(ys)
    return bytes(x ^ y for x, y in zip(xs, ys))

def keygen(ks):
    return random.randbytes(ks)

def ivgen(bs):
    return random.randbytes(bs)

def expand_key(params, key):
    ks, bs = params
    assert len(key) == ks
    R = Rijndael(key, bs)
    round_keys = []
    for ws in R.Ke:
        round_keys.append(pack(f'>{bs // 4}I', *ws))
        assert len(round_keys[-1]) == bs
    assert len(round_keys) == num_rounds(ks)
    assert round_keys[0][:min(ks, bs)] == key[:min(ks, bs)]
    return round_keys

def gen_keys():
    keys = defaultdict(list)
    for ks in [16, 24, 32]:
        for _ in range(num_keys):
            keys[ks].append(keygen(ks))
    return keys

keys = gen_keys()

def print_array(bs):
    return '{' + ', '.join(f'0x{b:02x}' for b in bs) + '}'

#  for ks, bs in params:
#      print((ks, bs))
#      for i, k in enumerate(keys[ks]):
#          rks = expand_key((ks, bs), k)
#          print('    {')
#          for rk in rks:
#              print(' ' * 8 + print_array(rk) + ',')
#          print('    },')

def aesctr(key, iv, counter):
    R = Rijndael(key)
    blocks = []
    iv = int.from_bytes(iv, 'big')
    for c in range(counter, counter + num_ctr_blocks):
        blocks.append(R.encrypt((iv + c).to_bytes(16, 'big')))
    return b''.join(blocks)


def gen_aes_ctr_tests(keys):
    ivs = [ivgen(16) for _ in range(num_keys)]
    counter = random.getrandbits(32)
    ctrblocks = defaultdict(list)
    for ks, aes_keys in keys.items():
        for iv, key in zip(ivs, aes_keys):
            ctrblocks[ks].append(aesctr(key, iv, counter))
    return ivs, counter, ctrblocks

ivs, counter, ctrblocks = gen_aes_ctr_tests(keys)

print('ivs')
for iv in ivs:
    print(print_array(iv))
print('counter')
print(counter)

for ks, outputs in ctrblocks.items():
    print(f'ks = {8 * ks}')
    for o in outputs:
        print('    ' + print_array(o) + ',')
