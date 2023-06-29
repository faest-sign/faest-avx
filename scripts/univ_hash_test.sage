
set_random_seed(42)

F2 = GF(2)
R2.<X> = F2[]

F64 = GF(2^64, name='z64', modulus=X^64 + X^4 + X^3 + X + 1)
F128 = GF(2^128, name='z128', modulus=X^128 + X^7 + X^2 + X + 1)
F192 = GF(2^192, name='z192', modulus=X^192 + X^7 + X^2 + X + 1)
F256 = GF(2^256, name='z256', modulus=X^256 + X^10 + X^5 + X^2 + 1)


def byte_len(F):
    return log(F.cardinality(), 2) / 8

def poly_to_int(a):
    return int(R2(a).change_ring(ZZ)(2))

def fe_to_bytes(byte_size, a):
    return poly_to_int(a).to_bytes(byte_size, 'little')

def bytes_to_fe(F, bs):
    assert len(bs) == byte_len(F)
    return F.from_integer(int.from_bytes(bs, 'little'))

def fe_list_to_cpp_array(byte_size, xs):
    return ', '.join(f'0x{a:02x}' for a in b''.join(map(lambda x: fe_to_bytes(byte_size, x), xs)))

n_hashers = 4
n_inputs = 8

def gen_tests(F):
    keys = [F.random_element() for _ in range(n_hashers)]
    inputs = [[F.random_element() for _ in range(n_inputs)] for _ in range(n_hashers)]
    outputs = [sum(key ^ i * a for i, a in enumerate(reversed(ins))) for key, ins in zip(keys, inputs)]
    return keys, inputs, outputs

def make_64_secpar_tests(F, keys64, inputs):
    keys = [F(R2(k)) for k in keys64]
    outputs = [sum(key ^ i * a for i, a in enumerate(reversed(ins))) for key, ins in zip(keys, inputs)]
    return outputs

def print_tests(F, t):
    k, i, o = t
    print("keys")
    for ks in k:
        print('    {' + fe_list_to_cpp_array(byte_len(F), [ks]) + '},')
    print("inputs")
    for ins in i:
        print('    {' + fe_list_to_cpp_array(byte_len(F), ins) + '},')
    print("outputs")
    for os in o:
        print('    {' + fe_list_to_cpp_array(byte_len(F), [os]) + '},')

t_128 = gen_tests(F128)
print("128")
print_tests(F128, t_128)
t_192 = gen_tests(F192)
print("192")
print_tests(F192, t_192)
t_256 = gen_tests(F256)
print("256")
print_tests(F256, t_256)
t_64 = gen_tests(F64)
print("64")
print_tests(F64, t_64)

mixed_64_128_outputs = make_64_secpar_tests(F128, t_64[0], t_128[1])
mixed_64_192_outputs = make_64_secpar_tests(F192, t_64[0], t_192[1])
mixed_64_256_outputs = make_64_secpar_tests(F256, t_64[0], t_256[1])
print('mixed 128')
for os in mixed_64_128_outputs:
    print('    {' + fe_list_to_cpp_array(byte_len(F128), [os]) + '},')
print('mixed 192')
for os in mixed_64_192_outputs:
    print('    {' + fe_list_to_cpp_array(byte_len(F192), [os]) + '},')
print('mixed 256')
for os in mixed_64_256_outputs:
    print('    {' + fe_list_to_cpp_array(byte_len(F256), [os]) + '},')
