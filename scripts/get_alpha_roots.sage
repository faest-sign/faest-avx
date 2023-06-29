#!/usr/bin/env sage

F2 = GF(2)
R2.<X> = F2[]

F8_mod = X^8 + X^4 + X^3 + X + 1

F64 = GF(2^64, name='z64', modulus=X^64 + X^4 + X^3 + X + 1)
F128 = GF(2^128, name='z128', modulus=X^128 + X^7 + X^2 + X + 1)
F192 = GF(2^192, name='z192', modulus=X^192 + X^7 + X^2 + X + 1)
F256 = GF(2^256, name='z256', modulus=X^256 + X^10 + X^5 + X^2 + 1)

def poly_to_int(a):
    return int(R2(a).change_ring(ZZ)(2))

def fe_to_bytes(byte_size, a):
    return poly_to_int(a).to_bytes(byte_size, 'little')

def fe_list_to_cpp_array(byte_size, xs):
    return '{' + '},\n{'.join(', '.join([f'0x{a:02x}' for a in fe_to_bytes(byte_size, x)]) for x in xs) + '}'

def print_root_powers(field):
    roots = [r for r, m in F8_mod.change_ring(field).roots()]
    for r0, r1 in zip(roots[:-1], roots[1:]):
        # Sage outputs in lexiographic order:
        assert(r0.to_integer() < r1.to_integer())

    # Arbitrarily pick the lexiographically first root.
    root = roots[0]
    print(fe_list_to_cpp_array((field.degree() + 7) // 8, [root^i for i in range(1, F8_mod.degree())]))

print("F64")
print_root_powers(F64)
print("F128")
print_root_powers(F128)
print("F192")
print_root_powers(F192)
print("F256")
print_root_powers(F256)
