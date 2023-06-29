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

n = 4 * 4

def gen_tests(F):
    xs = [F.random_element() for _ in range(n)]
    ys = [F.random_element() for _ in range(n)]
    sums = [a + b for a, b in zip(xs, ys)]
    products = [a * b for a, b in zip(xs, ys)]
    unreduced_products = [R2(a) * R2(b) for a, b in zip(xs, ys)]
    return xs, ys, sums, unreduced_products, products

def print_tests(F, t):
    xs, ys, sums, unreduced_products, products = t
    print(fe_list_to_cpp_array(byte_len(F), xs))
    print(fe_list_to_cpp_array(byte_len(F), ys))
    print(fe_list_to_cpp_array(byte_len(F), sums))
    print(fe_list_to_cpp_array(2 * byte_len(F), unreduced_products))
    print(fe_list_to_cpp_array(byte_len(F), products))

t_64 = gen_tests(F64)
print("F64")
print_tests(F64, t_64)

t_128 = gen_tests(F128)
print("F128")
print_tests(F128, t_128)

t_192 = gen_tests(F192)
print("F192")
print_tests(F192, t_192)

t_256 = gen_tests(F256)
print("F256")
print_tests(F256, t_256)
