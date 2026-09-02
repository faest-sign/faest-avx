#!/usr/bin/env python3
"""
vole_mult_tables.py

Search + table generation for the CRT-based F_{2^lambda} VOLE multiplication.

Model
-----
We want u * Delta in F_2[x]/p(x), deg p = lambda.  The mask u is a full
lambda-bit polynomial, but Delta is committed by its residues modulo the
tau GGM-tree moduli: a vector Delta' of n_delta_bits = lambda - w_grind
bits (w_grind is FAEST's grinding parameter; the tree degrees sum to
lambda - w_grind).  Delta itself is the CRT lift W_crt * Delta', the
unique representative of degree < lambda - w_grind.  The polynomial
product u_hat * Delta_hat therefore has degree <= 2*lambda - w_grind - 2,
so it is determined by its residues modulo any pairwise-coprime family
of "places" of total degree >= 2*lambda - w_grind - 1 (places = powers
of irreducibles, plus the place at infinity, whose multiplicity-m
residue is the top m product coefficients).

  * The tau GGM-tree moduli (distinct irreducibles, degrees summing to
    lambda - w_grind, as dictated by the FAEST parameter set) serve
    their residue products with zero bit-multiplication gates and zero
    correction bits: the residues of u are read directly off the trees.
  * Every remaining place is decomposed recursively (CRT / Karatsuba /
    short products) down to single bit multiplications
    f_e(u) * g_e(Delta').  Each such gate costs exactly one correction
    bit per mask (one garbled/hashed F_2-VOLE), so the per-mask
    signature cost is n_gates bits.
  * All linear steps (reductions, lifting, interpolation, recombination)
    are free; they are flattened into fixed public matrices F, G,
    W_tree, W_gate, W_crt.
  * The degree deficit covered by gates is
    (2*lambda - w_grind - 1) - (lambda - w_grind) = lambda - 1,
    independent of w_grind.

Cost search
-----------
full_cost(n): bilinear gates for the full product of two n-coefficient
polynomials; min over (a) exact multiple-choice knapsack of CRT places
(linear places x, x+1, infinity at any multiplicity via short products;
powers of x^2+x+1; distinct irreducibles of degree >= 3), (b)
Karatsuba-style splits full(s)*full(ceil(n/s)), and (c) hardcoded base
cases: Montgomery's 13-multiplication five-term formula (full_cost(5))
and, as place costs, the F_4-tower field multiplications (deg 6 -> 15,
deg 8 -> 24; see TOWER_FIELD).
short_cost(n): product mod x^n, via a split full(m) + 2*short(n - m)
minimized over all m >= ceil(n/2), or full-and-truncate.
The top-level portfolio for the degree deficit D = lambda - 1 is an
exact knapsack over the same place inventory (tree moduli excluded, so
the achievable gate count can differ slightly between variants that
consume different irreducibles in their trees).  With Montgomery's
five-term formula and the F_4-tower degree-6/8 constructions built in,
the constructive costs match the best published bilinear complexities
at every degree in the inventory.

Table generation
----------------
Bilinear algorithms are built compositionally as (gates, W) objects with
independent left/right input widths (u has lambda coefficients, Delta_hat
has lambda - w_grind) and verified two ways: --selftest checks the small
building blocks against schoolbook multiplication, and every generated
table set is checked on all lambda * (lambda - w_grind) basis pairs
(e_i, e_j') against direct arithmetic in F_2[x]/p(x) -- by bilinearity
this is a complete proof of the identity.
Output: JSON tables (F, G, W_tree, W_gate, W_crt, the tree moduli M_i and
their product M_tree, tree reduction matrices, portfolio metadata) and
optionally a C header/source pair (extern declarations in the .h, one
definition of each table in the .c, so nothing is duplicated across
translation units).

Table semantics
---------------------------------------------
A table set is (lambda, w_grind, p, M_0..M_{tau-1}, M_tree, F, G, W_tree,
W_gate, W_crt) and asserts exactly one bilinear identity.
Conventions: a vector v in F_2^k is an integer with bit i = v_i (the JSON
stores these integers as hex strings);
a polynomial of degree < k is identified with its coefficient vector,
bit i = coefficient of x^i -- the spec's ToPoly / ToBits; <r, v> denotes
the F_2 inner product parity(r AND v), written in the spec as the
row-times-column product r * v.  Names follow the spec section "VOLE
Commitment Masks with CRT": Delta' / Delta, u_low / u_hi, res_tau, and the
gate index e.

For u in F_2^lambda and Delta' in F_2^{lambda - w_grind}, let
Delta = W_crt * Delta' in F_2^lambda (rows of W_crt are masks over the
Delta' bits) and define:

  * the tree residue vector r(u, Delta') in F_2^{n_tree_bits}, the
    spec's res_tau(u, Delta): for each tree modulus M_i
    (i = 0..tau-1, in the order of tree_moduli_hex) let
        s_i = (u mod M_i) * (Delta mod M_i) mod M_i,
    a deg(M_i)-bit vector; r is the concatenation s_0 || ... || s_{tau-1},
    with bit b of s_i placed at offset deg(M_0)+...+deg(M_{i-1}) + b.
    Note Delta mod M_i is exactly the slice of Delta' belonging to tree
    i.  (In the protocol these residue products are supplied by the GGM
    trees; the identity does not care where they come from.)

  * the gate vector g(u, Delta') in F_2^{n_gates}, indexed by the gate
    index e:
        g_e = <F[e], u> * <G[e], Delta'>,
    i.e. the componentwise product (F u) * (G Delta') of the two linear
    maps f_e, g_e.

The identity: for every output index k in 0..lambda-1,

    coefficient k of (u * Delta mod p)
        = <W_tree[k], r(u, Delta')>  XOR  <W_gate[k], g(u, Delta')>,

where u * Delta is the plain product in F_2[x] and p is modulus_hex.
In the matrix form used by the spec,

    u * Delta = W_tree * res_tau(u, Delta) + W_gate * (F u * G Delta').

M_tree = M_0*...*M_{tau-1} (degree lambda - w_grind) is not part of the
identity: it is used by the prover to assemble a full-field mask
u = W_crt * u_low + M_tree(x) * ToPoly(u_hi) from the tree residue bits
u_low and the w_grind extra secret bits u_hi.

Verification: both sides are bilinear in (u, Delta'), so the identity
holds for all input pairs if and only if it holds on the
lambda * (lambda - w_grind) basis pairs (u, Delta') = (x^i, e_j').
Checking all of them is therefore a complete proof; it needs only F_2[x]
arithmetic and popcounts, and --verify implements it from the JSON
alone, with no knowledge of how the tables were constructed: it obtains
Delta by multiplying Delta' with the stored W_crt, and separately checks
(again from the stored tables only) that W_crt is the CRT lift of the
tree moduli -- column j is x^b mod M_i and 0 mod every other modulus --
and that M_tree is their product.  The field tree_reduction_rows is
redundant convenience
data for implementers (the rows of the linear maps u -> u mod M_i over
the lambda input bits); it can be recomputed from tree_moduli_hex and is
not part of the identity.

Parameter provenance: modulus_hex is FAEST's field polynomial P_lambda
(see FAEST_MODULI below); tau and w_grind per preset are the FAEST
round-3 parameters; the tree degrees follow FAEST's small-VOLE sizes
(tau_1 trees of depth k = floor((lambda-w)/tau)+1 and tau_0 = tau - tau_1
of depth k-1, where tau_1 = (lambda-w) mod tau may be 0); the spec's
per-tree depth is d_i = k for i < tau_1 and k-1 otherwise, so the largest
depth is d_0, which equals k only when tau_1 > 0; the tree moduli are the
lexicographically smallest
distinct irreducibles of those degrees, a choice the protocol spec must
pin identically.

Only the search is clever; consumers only ever need the fixed tables
plus the basis-pair checker above.
"""

import argparse
import functools
import json
import os
import random

# ----------------------------------------------------------------------
# GF(2)[x] arithmetic on ints (bit i = coefficient of x^i)
# ----------------------------------------------------------------------

def pdeg(a):
    return a.bit_length() - 1

def pmul(a, b):
    r = 0
    while b:
        if b & 1:
            r ^= a
        b >>= 1
        a <<= 1
    return r

def pmod(a, m):
    d = pdeg(m)
    while a and pdeg(a) >= d:
        a ^= m << (pdeg(a) - d)
    return a

def pmulmod(a, b, m):
    return pmod(pmul(a, b), m)

def pgcd(a, b):
    while b:
        a, b = b, pmod(a, b)
    return a

def pdivmod(a, b):
    q, db = 0, pdeg(b)
    while a and pdeg(a) >= db:
        s = pdeg(a) - db
        q |= 1 << s
        a ^= b << s
    return q, a

def pinvmod(a, m):
    """Inverse of a mod m in F_2[x] (extended Euclid); requires gcd = 1."""
    r0, r1 = m, pmod(a, m)
    s0, s1 = 0, 1
    while r1:
        q, r = pdivmod(r0, r1)
        r0, r1 = r1, r
        s0, s1 = s1, s0 ^ pmul(q, s1)
    assert r0 == 1, "pinvmod: not coprime"
    return pmod(s0, m)

def crt_lift_cols(tree_moduli):
    """(cols, M_tree): cols[j] is the CRT lift of the j-th tree residue
    basis bit -- the unique polynomial of degree < sum(deg M_i) congruent
    to x^b mod its own tree modulus and 0 mod all others -- ordered as the
    residue vector (tree by tree, bit by bit).  These are the columns of
    W_crt.  M_tree is the product of the tree moduli."""
    Mtree = 1
    for M in tree_moduli:
        Mtree = pmul(Mtree, M)
    cols = []
    for M in tree_moduli:
        Q, rem = pdivmod(Mtree, M)
        assert rem == 0
        E = pmulmod(Q, pinvmod(Q, M), Mtree)  # CRT idempotent for M
        for b in range(pdeg(M)):
            cols.append(pmulmod(E, 1 << b, Mtree))
    return cols, Mtree

def mtree_poly(tree_moduli):
    """Product M_tree(x) = prod_i M_i(x) of the tree moduli, of degree
    sum(deg M_i) = lambda - wgrind."""
    M = 1
    for q in tree_moduli:
        M = pmul(M, q)
    return M

def poly_pow(p, e):
    r = 1
    for _ in range(e):
        r = pmul(r, p)
    return r

def _x_pow_2k(f, k):
    """x^(2^k) mod f."""
    r = pmod(2, f)
    for _ in range(k):
        r = pmulmod(r, r, f)
    return r

def _prime_factors(n):
    fs, d = set(), 2
    while d * d <= n:
        while n % d == 0:
            fs.add(d)
            n //= d
        d += 1
    if n > 1:
        fs.add(n)
    return fs

def is_irreducible(f):
    n = pdeg(f)
    if n <= 0:
        return False
    if n == 1:
        return True
    if not (f & 1):
        return False  # divisible by x
    if _x_pow_2k(f, n) != pmod(2, f):
        return False
    for q in _prime_factors(n):
        h = pmod(_x_pow_2k(f, n // q) ^ 2, f)
        if pgcd(f, h) != 1:
            return False
    return True

@functools.lru_cache(maxsize=None)
def irreducibles_of_degree(d):
    return tuple(f for f in range(1 << d, 1 << (d + 1)) if is_irreducible(f))

# Field moduli pinned to the FAEST specification's field polynomials:
#   P_128(x) = x^128 + x^7  + x^2 + x + 1   (also the AES-GCM polynomial)
#   P_192(x) = x^192 + x^7  + x^2 + x + 1
#   P_256(x) = x^256 + x^10 + x^5 + x^2 + 1
# Each also happens to be what the fallback search below returns (the
# lexicographically first irreducible trinomial/pentanomial of its
# degree), but the tables must match FAEST exactly, so they are pinned
# rather than searched.  Irreducibility is re-asserted at runtime.
FAEST_MODULI = {
    128: (1 << 128) | (1 << 7) | (1 << 2) | (1 << 1) | 1,
    192: (1 << 192) | (1 << 7) | (1 << 2) | (1 << 1) | 1,
    256: (1 << 256) | (1 << 10) | (1 << 5) | (1 << 2) | 1,
}

def find_modulus(lam):
    """FAEST field polynomial for lambda in {128, 192, 256}; otherwise the
    first irreducible trinomial, then pentanomial, of degree lam."""
    if lam in FAEST_MODULI:
        m = FAEST_MODULI[lam]
        assert is_irreducible(m)
        return m
    base = 1 << lam
    for a in range(1, lam):
        m = base | (1 << a) | 1
        if is_irreducible(m):
            return m
    for a in range(3, lam):
        for b in range(2, a):
            for c in range(1, b):
                m = base | (1 << a) | (1 << b) | (1 << c) | 1
                if is_irreducible(m):
                    return m
    raise RuntimeError("no low-weight irreducible found")

# ----------------------------------------------------------------------
# helpers: bit iteration, F_2 row combination, matrices as int rows
# ----------------------------------------------------------------------

def bits_iter(x):
    while x:
        b = x & -x
        yield b.bit_length() - 1
        x ^= b

def combine(mask, rows):
    """XOR of rows[i] over set bits i of mask."""
    out = 0
    for i in bits_iter(mask):
        out ^= rows[i]
    return out

def parity(x):
    return x.bit_count() & 1

def reduction_rows(q, ncols):
    """Rows of the linear map (c_0..c_{ncols-1}) -> coefficients of
    (sum c_j x^j) mod q.  Row i is a mask over the ncols inputs."""
    d = pdeg(q)
    rows = [0] * d
    cur = 1
    for j in range(ncols):
        for i in bits_iter(cur):
            rows[i] |= 1 << j
        cur = pmod(cur << 1, q)
    return rows

def pascal_rows(e):
    """Substitution x -> x+1 on polynomials of degree < e:
    out_i = sum_j C(j,i) in_j; C(j,i) odd iff i is a submask of j (Lucas).
    Involution over F_2."""
    return [sum(1 << j for j in range(e) if (i & j) == i) for i in range(e)]

def left_inverse(rows, ncols):
    """Given injective rows (masks over ncols columns), return X with
    X . rows = I: X[c] is a mask over the original row indices."""
    m = len(rows)
    work = [[rows[i], 1 << i] for i in range(m)]
    used = [False] * m
    piv_of_col = [None] * ncols
    for c in range(ncols):
        piv = None
        for i in range(m):
            if not used[i] and (work[i][0] >> c) & 1:
                piv = i
                break
        if piv is None:
            raise AssertionError("evaluation map not full rank (bad portfolio?)")
        used[piv] = True
        piv_of_col[c] = piv
        pr, pt = work[piv]
        for i in range(m):
            if i != piv and (work[i][0] >> c) & 1:
                work[i][0] ^= pr
                work[i][1] ^= pt
    return [work[piv_of_col[c]][1] for c in range(ncols)]

# ----------------------------------------------------------------------
# Tower construction over F_4: constructive 15-gate (deg 6) and 24-gate
# (deg 8) field multiplications.
#
# F_{2^{2m}} = F_4[x]/Q(x) with deg Q = m.  The full product of two m-term
# polynomials over F_4 is computed by CRT over places of P^1(F_4):
#   m=3: evaluate at 0, 1, w, w^2, infinity        -> 5 F_4-mults (= 2m-1,
#        the Winograd/de Groote floor, reachable since m <= q/2+1)
#   m=4: those 5 places + one quadratic place      -> 8 F_4-mults
#        (rank-8 like Baum-Shokrollahi's Fermat-curve algorithm, but via a
#        degree-2 place of the projective line instead of an elliptic curve)
# Each F_4-multiplication costs 3 bit-multiplications (Karatsuba), and all
# F_4-linear glue (evaluation, interpolation, reduction mod Q, the field
# isomorphism to the standard basis of the target modulus) is F_2-linear,
# hence free.  Totals: mu_2(6) <= 3*5 = 15, mu_2(8) <= 3*8 = 24.
# ----------------------------------------------------------------------

# F_4 = F_2[w]/(w^2+w+1); elements coded 0,1,2,3 as u+2v for u+v*w.
# Addition is XOR of codes.
_F4M = [[0, 0, 0, 0], [0, 1, 2, 3], [0, 2, 3, 1], [0, 3, 1, 2]]

def f4_mul(a, b):
    return _F4M[a][b]

def f4_pow(t, j):
    r = 1
    for _ in range(j):
        r = f4_mul(r, t)
    return r

def f4_pmul(A, B):
    out = [0] * (len(A) + len(B) - 1)
    for i, a in enumerate(A):
        if a:
            for j, b in enumerate(B):
                out[i + j] ^= f4_mul(a, b)
    return out

def f4_pmod(A, Q):
    """A mod monic Q, both coefficient lists over F_4."""
    dq = len(Q) - 1
    work = list(A)
    for i in range(len(work) - 1, dq - 1, -1):
        c = work[i]
        if c:
            for j in range(dq + 1):
                work[i - dq + j] ^= f4_mul(c, Q[j])
    return (work + [0] * dq)[:dq]

def f4_peval(P, t):
    r = 0
    for c in reversed(P):
        r = f4_mul(r, t) ^ c
    return r

def f4_irreducibles(m):
    """Monic irreducibles of degree m over F_4 (m <= 4 supported)."""
    import itertools
    quads = None
    if m == 4:
        quads = [list(t) + [1] for t in itertools.product(range(4), repeat=2)
                 if all(f4_peval(list(t) + [1], u) for u in range(4))]
    out = []
    for t in itertools.product(range(4), repeat=m):
        P = list(t) + [1]
        if any(f4_peval(P, u) == 0 for u in range(4)):
            continue
        if m == 4 and any(not any(f4_pmod(P, Q2)) for Q2 in quads):
            continue
        out.append(P)
    return out

def f4_xpow_cols(Q, count):
    """Columns x^j mod Q for j < count, each a length-(deg Q) list."""
    dq = len(Q) - 1
    cur = [1] + [0] * (dq - 1)
    cols = []
    for _ in range(count):
        cols.append(list(cur))
        cur = f4_pmod([0] + cur, Q)
    return cols

def f4_matinv(M):
    n = len(M)
    _F4INV = [None, 1, 3, 2]
    A = [list(M[i]) + [1 if j == i else 0 for j in range(n)] for i in range(n)]
    for c in range(n):
        piv = next(i for i in range(c, n) if A[i][c])
        A[c], A[piv] = A[piv], A[c]
        inv = _F4INV[A[c][c]]
        A[c] = [f4_mul(inv, v) for v in A[c]]
        for i in range(n):
            if i != c and A[i][c]:
                f = A[i][c]
                A[i] = [A[i][j] ^ f4_mul(f, A[c][j]) for j in range(2 * n)]
    return [row[n:] for row in A]

def _f4_full_product_alg(m):
    """(gates4, W4): gates4[l] is a length-m F_4-linear form (applied to both
    inputs); W4 is the (2m-1) x len(gates4) F_4-matrix recombining the
    F_4-multiplication outputs into the full product coefficients."""
    gates4, ev, R4 = [], [], []
    npts = 2 if m == 2 else 4
    for t in range(npts):
        R4.append([(len(gates4), 1)])
        gates4.append([f4_pow(t, j) for j in range(m)])
        ev.append([f4_pow(t, j) for j in range(2 * m - 1)])
    R4.append([(len(gates4), 1)])                       # infinity
    gates4.append([0] * (m - 1) + [1])
    row = [0] * (2 * m - 1)
    row[2 * m - 2] = 1
    ev.append(row)
    if m == 4:                                          # one quadratic place
        Q2 = f4_irreducibles(2)[0]
        beta, alpha = Q2[0], Q2[1]
        cols_in = f4_xpow_cols(Q2, m)
        r0 = [c[0] for c in cols_in]
        r1 = [c[1] for c in cols_in]
        ga = len(gates4)
        gates4 += [r0, r1, [a ^ b for a, b in zip(r0, r1)]]
        cols_out = f4_xpow_cols(Q2, 2 * m - 1)
        ev.append([c[0] for c in cols_out])
        ev.append([c[1] for c in cols_out])
        # Karatsuba in F_4[x]/Q2: c0=g_a, c2=g_b, c1=g_a+g_b+g_c; then
        # x^2 = alpha*x + beta gives d0 = c0 + beta*c2, d1 = c1 + alpha*c2
        R4.append([(ga, 1), (ga + 1, beta)])
        R4.append([(ga, 1), (ga + 1, 1 ^ alpha), (ga + 2, 1)])
    Inv = f4_matinv(ev)
    ng = len(gates4)
    W4 = [[0] * ng for _ in range(2 * m - 1)]
    for k in range(2 * m - 1):
        for j, entries in enumerate(R4):
            c = Inv[k][j]
            if c:
                for g, coeff in entries:
                    W4[k][g] ^= f4_mul(c, coeff)
    return gates4, W4

def _t_to_bits(t):
    return sum(t[j] << (2 * j) for j in range(len(t)))

def tower_place_alg(q, na, nb):
    """24-gate (deg 8) / 15-gate (deg 6) algorithm for the residue of the
    product mod irreducible q, from an na- and an nb-coefficient input."""
    d = pdeg(q)
    m = d // 2
    Q = f4_irreducibles(m)[0]
    gates4, W4 = _f4_full_product_alg(m)
    # reduce the full 2m-1 coefficient product mod Q (F_4-linear, free)
    RQ = f4_xpow_cols(Q, 2 * m - 1)      # RQ[j][k] = coeff k of x^j mod Q
    ng4 = len(gates4)
    W4m = [[0] * ng4 for _ in range(m)]
    for k in range(m):
        for j in range(2 * m - 1):
            c = RQ[j][k]
            if c:
                for g in range(ng4):
                    W4m[k][g] ^= f4_mul(c, W4[j][g])
    # field isomorphism phi: F_2[y]/q -> F_4[x]/Q via a root rho of q
    import itertools
    one = tuple([1] + [0] * (m - 1))

    def tmul(a, b):
        return tuple(f4_pmod(f4_pmul(list(a), list(b)), Q))

    rho = None
    for cand in itertools.product(range(4), repeat=m):
        acc = (0,) * m
        for i in range(pdeg(q), -1, -1):
            acc = tmul(acc, cand)
            if (q >> i) & 1:
                acc = tuple(x ^ y for x, y in zip(acc, one))
        if not any(acc):
            rho = cand
            break
    assert rho is not None, "no root of q in the tower field?!"
    cols, p = [], one
    for _ in range(d):
        cols.append(_t_to_bits(p))
        p = tmul(p, rho)
    Mrows = [sum(((cols[i] >> r) & 1) << i for i in range(d)) for r in range(d)]
    Minv = left_inverse(Mrows, d)
    # input side: reduce mod q, map to tower coordinates (per operand)
    trow_a = [combine(Mrows[r], reduction_rows(q, na)) for r in range(d)]
    trow_b = [combine(Mrows[r], reduction_rows(q, nb)) for r in range(d)]

    def form_bits(L, trow):
        p_ = r_ = 0
        for j, c in enumerate(L):
            if c:
                pj, rj = trow[2 * j], trow[2 * j + 1]
                if c & 1:                # + 1 * (pj + rj*w)
                    p_ ^= pj
                    r_ ^= rj
                if c & 2:                # + w * (pj + rj*w)
                    p_ ^= rj
                    r_ ^= pj ^ rj
        return p_, r_

    gates, comp = [], []
    for L in gates4:
        pa, ra = form_bits(L, trow_a)
        pb, rb = form_bits(L, trow_b)
        b = len(gates)
        gates += [(pa, pb), (ra, rb), (pa ^ ra, pb ^ rb)]
        comp.append(((1 << b) | (1 << (b + 1)),      # comp0 = g0+g1
                     (1 << b) | (1 << (b + 2))))     # comp1 = g0+g2
    towout = [0] * d
    for k in range(m):
        o0 = o1 = 0
        for g in range(ng4):
            c = W4m[k][g]
            if c:
                G0, G1 = comp[g]
                if c & 1:
                    o0 ^= G0
                    o1 ^= G1
                if c & 2:
                    o0 ^= G1
                    o1 ^= G0 ^ G1
        towout[2 * k], towout[2 * k + 1] = o0, o1
    W = [combine(Minv[r], towout) for r in range(d)]
    return Alg(na, nb, gates, W), reduction_rows(q, na + nb - 1)

# Field degrees served by the tower construction, with their gate costs.
TOWER_FIELD = {6: 15, 8: 24}

# ----------------------------------------------------------------------
# Montgomery's five-term formula: 13 bit-multiplications for the full
# product of two 5-term polynomials (P.L. Montgomery, "Five, six, and
# seven-term Karatsuba-like formulae", IEEE Trans. Computers 54(3), 2005;
# transcribed from the explicit statement in US patent 7,765,252 and
# reduced mod 2; proven optimal for this size by Barbulescu-Detrey-
# Estibals-Zimmermann 2012).  The 13 products, as index sets over the
# five input coefficients (identical form on both sides):
#   p01234 p0234 p0124 p0134 p023 p124 p34 p01 p04 p4 p3 p1 p0
# The transcription is machine-verified against schoolbook multiplication
# on all basis pairs in --selftest (and transitively by every table
# verification), so a transcription error cannot survive silently.
# ----------------------------------------------------------------------

_MONT5_GATES = [0b11111, 0b11101, 0b10111, 0b11011, 0b01101, 0b10110,
                0b11000, 0b00011, 0b10001, 0b10000, 0b01000, 0b00010,
                0b00001]
_MONT5_W = [  # product coefficients c0..c8 as gate index sets, mod 2
    [12],
    [7, 11, 12],
    [2, 5, 7, 8, 9, 12],
    [0, 1, 3, 6, 8, 9],
    [0, 4, 5, 6, 7, 9, 10, 11, 12],
    [0, 2, 3, 7, 8, 12],
    [1, 4, 6, 8, 9, 12],
    [6, 9, 10],
    [9],
]

def mont5_alg():
    gates = [(m, m) for m in _MONT5_GATES]
    W = [sum(1 << g for g in row) for row in _MONT5_W]
    return Alg(5, 5, gates, W)

# ----------------------------------------------------------------------
# Cost search
# ----------------------------------------------------------------------

@functools.lru_cache(maxsize=None)
def full_cost(n):
    """(gates, plan) for the full product of two n-coefficient polys."""
    if n == 1:
        return (1, ('base',))
    best = (13, ('mont5',)) if n == 5 else None
    for s in range(2, n):
        m = -(-n // s)
        c = full_cost(s)[0] * full_cost(m)[0]
        if best is None or c < best[0]:
            best = (c, ('split', s, m))
    pools = {d: list(irreducibles_of_degree(d)) for d in range(2, n)}
    res = crt_search(2 * n - 1, maxe_lin=n - 1, cap_irr=n - 1,
                     irr_pools=pools)
    if res is not None:
        c, picks = res
        if best is None or c < best[0]:
            best = (c, ('crt', tuple(picks)))
    assert best is not None
    return best

@functools.lru_cache(maxsize=None)
def short_cost(n):
    """(gates, plan) for the product mod x^n of two n-coefficient polys."""
    if n == 1:
        return (1, ('base',))
    best = (full_cost(n)[0], ('full',))
    for m in range(-(-n // 2), n):  # any m with 2m >= n kills the x^{2m} A1*B1 term mod x^n
        c = full_cost(m)[0] + 2 * short_cost(n - m)[0]
        if c < best[0]:
            best = (c, ('shortsplit', m))
    return best

def place_gate_cost(kind, poly, e):
    if kind in ('inf', 'lin'):
        return short_cost(e)[0]
    d = pdeg(poly)
    if e == 1 and d in TOWER_FIELD:
        return min(TOWER_FIELD[d], full_cost(d)[0])
    return full_cost(d * e)[0]

def crt_search(target, maxe_lin, cap_irr, irr_pools):
    """Exact multiple-choice knapsack: pick at most one multiplicity per
    place, total degree >= target, minimize total gate cost.
    Returns (cost, picks) with picks a list of (kind, poly, e)."""
    groups = []
    for kind, poly in (('inf', None), ('lin', 0b10), ('lin', 0b11)):
        opts = [(e, short_cost(e)[0], [(kind, poly, e)])
                for e in range(1, maxe_lin + 1)]
        if opts:
            groups.append(opts)
    for d in sorted(irr_pools):
        pool = irr_pools[d]
        if not pool:
            continue
        if d == 2:
            # only one degree-2 irreducible exists; allow its powers
            p = pool[0]
            opts = []
            e = 1
            while d * e <= cap_irr:
                opts.append((d * e, place_gate_cost('irr', p, e),
                             [('irr', p, e)]))
                e += 1
            if opts:
                groups.append(opts)
        else:
            per = place_gate_cost('irr', pool[0], 1)
            maxt = min(len(pool), target // d + 1)
            opts = [(t * d, t * per, [('irr', pool[i], 1) for i in range(t)])
                    for t in range(1, maxt + 1)]
            if opts:
                groups.append(opts)
    dp = {0: (0, [])}
    for opts in groups:
        ndp = dict(dp)
        for deg0, (c0, pk0) in dp.items():
            for dd, cc, pks in opts:
                nd = min(deg0 + dd, target)
                nc = c0 + cc
                cur = ndp.get(nd)
                if cur is None or nc < cur[0]:
                    ndp[nd] = (nc, pk0 + pks)
        dp = ndp
    return dp.get(target)

# ----------------------------------------------------------------------
# Bilinear algorithm objects and constructors
# ----------------------------------------------------------------------

class Alg:
    """Bilinear algorithm: gates[l] = (fa_mask, fb_mask) are linear forms
    over the two inputs; W[q] is a mask over gates giving output coord q."""
    __slots__ = ('na', 'nb', 'gates', 'W')

    def __init__(self, na, nb, gates, W):
        self.na, self.nb, self.gates, self.W = na, nb, gates, W

def precompose(alg, Ar, Br, na, nb):
    gates = [(combine(fa, Ar), combine(fb, Br)) for fa, fb in alg.gates]
    return Alg(na, nb, gates, list(alg.W))

def eval_alg(alg, a, b):
    gm = 0
    for i, (fa, fb) in enumerate(alg.gates):
        if parity(fa & a) and parity(fb & b):
            gm |= 1 << i
    out = 0
    for q, w in enumerate(alg.W):
        if parity(w & gm):
            out |= 1 << q
    return out

_FULL_ALG = {}
_SHORT_ALG = {}

def full_alg(n):
    if n in _FULL_ALG:
        return _FULL_ALG[n]
    _, plan = full_cost(n)
    if plan[0] == 'base':
        alg = Alg(1, 1, [(1, 1)], [1])
    elif plan[0] == 'mont5':
        alg = mont5_alg()
    elif plan[0] == 'split':
        s, m = plan[1], plan[2]
        OA, IA = full_alg(s), full_alg(m)
        ngi = len(IA.gates)
        gates = []
        for Fo, Go in OA.gates:
            Ar, Br = [0] * m, [0] * m
            for i in range(m):
                fa = fb = 0
                for t in bits_iter(Fo):
                    if t * m + i < n:
                        fa |= 1 << (t * m + i)
                for t in bits_iter(Go):
                    if t * m + i < n:
                        fb |= 1 << (t * m + i)
                Ar[i], Br[i] = fa, fb
            for fi, gi in IA.gates:
                gates.append((combine(fi, Ar), combine(gi, Br)))
        W = []
        for q in range(2 * n - 1):
            row = 0
            for r in range(2 * s - 1):
                i = q - r * m
                if 0 <= i <= 2 * m - 2:
                    for o in bits_iter(OA.W[r]):
                        row ^= IA.W[i] << (o * ngi)
            W.append(row)
        alg = Alg(n, n, gates, W)
    else:  # crt
        gates, resmasks, evrows = [], [], []
        for pk in plan[1]:
            palg, ev = place_alg(pk, n, n)
            off = len(gates)
            gates += palg.gates
            resmasks += [w << off for w in palg.W]
            evrows += ev
        X = left_inverse(evrows, 2 * n - 1)
        W = [combine(X[q], resmasks) for q in range(2 * n - 1)]
        alg = Alg(n, n, gates, W)
    _FULL_ALG[n] = alg
    return alg

def short_alg(n):
    if n in _SHORT_ALG:
        return _SHORT_ALG[n]
    _, plan = short_cost(n)
    if plan[0] == 'base':
        alg = Alg(1, 1, [(1, 1)], [1])
    elif plan[0] == 'full':
        A = full_alg(n)
        alg = Alg(n, n, list(A.gates), A.W[:n])
    else:  # shortsplit m
        m = plan[1]
        lo = [1 << i for i in range(m)]
        G1 = precompose(full_alg(m), lo, lo, n, n)
        S = short_alg(n - m)
        lo2 = [1 << i for i in range(n - m)]
        hi2 = [1 << (m + i) for i in range(n - m)]
        G2 = precompose(S, lo2, hi2, n, n)
        G3 = precompose(S, hi2, lo2, n, n)
        o2 = len(G1.gates)
        o3 = o2 + len(G2.gates)
        gates = G1.gates + G2.gates + G3.gates
        W = []
        for j in range(n):
            row = 0
            if j <= 2 * m - 2:
                row ^= G1.W[j]
            if j >= m:
                t = j - m
                row ^= (G2.W[t] << o2) ^ (G3.W[t] << o3)
            W.append(row)
        alg = Alg(n, n, gates, W)
    _SHORT_ALG[n] = alg
    return alg

def place_alg(pk, na, nb):
    """Algorithm computing the residue of the product at place pk, from an
    na- and an nb-coefficient input; plus the rows of the residue map over
    the (na+nb-1)-coefficient product (for global interpolation)."""
    kind, poly, e = pk
    N2 = na + nb - 1
    if kind == 'inf':
        assert e <= min(na, nb)
        rows_a = [1 << (na - 1 - i) for i in range(e)]
        rows_b = [1 << (nb - 1 - i) for i in range(e)]
        alg = precompose(short_alg(e), rows_a, rows_b, na, nb)
        ev = [1 << (N2 - 1 - i) for i in range(e)]
        return alg, ev
    if kind == 'lin':
        q = poly_pow(poly, e)
        Ra = reduction_rows(q, na)
        Rb = reduction_rows(q, nb)
        Rev = reduction_rows(q, N2)
        if poly == 0b10:  # x^e
            ina, inb, ev = Ra, Rb, Rev
        else:             # (x+1)^e, via substitution y = x+1
            P = pascal_rows(e)
            ina = [combine(P[i], Ra) for i in range(e)]
            inb = [combine(P[i], Rb) for i in range(e)]
            ev = [combine(P[i], Rev) for i in range(e)]
        return precompose(short_alg(e), ina, inb, na, nb), ev
    # irreducible power p^e
    if e == 1 and pdeg(poly) in TOWER_FIELD and \
            TOWER_FIELD[pdeg(poly)] < full_cost(pdeg(poly))[0]:
        return tower_place_alg(poly, na, nb)
    # generic: full product of the two d*e-coeff residues, reduce (free)
    q = poly_pow(poly, e)
    d = pdeg(q)
    FA = full_alg(d)
    alg = precompose(FA, reduction_rows(q, na), reduction_rows(q, nb),
                     na, nb)
    Rout = reduction_rows(q, 2 * d - 1)
    alg = Alg(na, nb, alg.gates, [combine(Rout[i], alg.W) for i in range(d)])
    return alg, reduction_rows(q, N2)

# ----------------------------------------------------------------------
# Top-level build, pruning, verification
# ----------------------------------------------------------------------

def build_tables(lam, wgrind, p, tree_moduli, picks):
    """F, G, W_tree, W_gate for inputs u in F_2^lam and Delta' in
    F_2^{lam-wgrind} (the tree residue bits of Delta); internally Delta is
    the lam-wgrind-coefficient lift, and G is composed down to the Delta'
    basis before returning."""
    na, nb = lam, lam - wgrind
    ntree = sum(pdeg(M) for M in tree_moduli)
    assert ntree == nb, "tree degrees must sum to lambda - wgrind"
    N2 = na + nb - 1
    evrows = []
    for M in tree_moduli:
        evrows += reduction_rows(M, N2)
    gates, resmasks, report = [], [], []
    for pk in picks:
        palg, ev = place_alg(pk, na, nb)
        off = len(gates)
        gates += palg.gates
        resmasks += [w << off for w in palg.W]
        evrows += ev
        report.append((pk, len(palg.gates)))
    X = left_inverse(evrows, N2)
    Red = reduction_rows(p, N2)
    Wt, Wg = [0] * lam, [0] * lam
    for r in range(lam):
        sel = combine(Red[r], X)
        for idx in bits_iter(sel):
            if idx < ntree:
                Wt[r] |= 1 << idx
            else:
                Wg[r] ^= resmasks[idx - ntree]
    F = [fa for fa, _ in gates]
    # G was built over the nb coefficients of the lifted Delta; compose with
    # the lift so its rows act on the residue bits Delta' directly.
    cols, _ = crt_lift_cols(tree_moduli)
    G = [sum(parity(fb & cols[j]) << j for j in range(nb))
         for _, fb in gates]
    return F, G, Wt, Wg, ntree, report

def wcrt_rows(tree_moduli, lam):
    """Rows of W_crt in F_2^{lam x (lam-wgrind)}: the CRT lift from tree
    residue bits to the coefficient vector of the degree < lam-wgrind
    representative (rows above that degree are zero)."""
    cols, _ = crt_lift_cols(tree_moduli)
    return [sum(((cols[j] >> r) & 1) << j for j in range(len(cols)))
            for r in range(lam)]

def prune(F, G, Wg):
    """Drop gates that are identically zero (padding artifacts), merge exact
    duplicates, and drop gates never referenced by W."""
    ng = len(F)
    canon = {}
    for g in range(ng):
        if F[g] and G[g]:
            canon.setdefault((F[g], G[g]), g)
    Wg1 = []
    for row in Wg:
        nr = 0
        for g in bits_iter(row):
            if F[g] == 0 or G[g] == 0:
                continue
            nr ^= 1 << canon[(F[g], G[g])]
        Wg1.append(nr)
    used = 0
    for row in Wg1:
        used |= row
    newidx, F2, G2 = {}, [], []
    for g in range(ng):
        if (used >> g) & 1:
            newidx[g] = len(F2)
            F2.append(F[g])
            G2.append(G[g])
    Wg2 = []
    for row in Wg1:
        nr = 0
        for g in bits_iter(row):
            nr |= 1 << newidx[g]
        Wg2.append(nr)
    return F2, G2, Wg2

def verify_tables(lam, wgrind, p, tree_moduli, F, G, Wt, Wg, Wcrt,
                  n_random=200, quick=False):
    """Check the bilinear identity on all lam * (lam-wgrind) basis pairs
    (u, Delta') = (x^i, e_j) -- complete by bilinearity -- plus random
    dense pairs.  Independent of construction: Delta = W_crt * Delta' is
    obtained by multiplying with the given W_crt table (whose column j is
    the lift of the j-th residue bit), so the check exercises the stored
    W_crt rather than re-deriving the lift."""
    nb = lam - wgrind
    degs = [pdeg(M) for M in tree_moduli]
    # columns of the supplied W_crt: cols[j] = W_crt * e_j, read straight
    # from the table's rows (bit r of cols[j] = W_crt[r][j]).
    cols = [sum(((Wcrt[r] >> j) & 1) << r for r in range(lam))
            for j in range(nb)]

    def tree_vec(a, bhat):
        vec, off = 0, 0
        for t, M in enumerate(tree_moduli):
            rr = pmulmod(pmod(a, M), pmod(bhat, M), M)
            vec |= rr << off
            off += degs[t]
        return vec

    def recombine(vec, gm):
        out = 0
        for r in range(lam):
            if parity(Wt[r] & vec) ^ parity(Wg[r] & gm):
                out |= 1 << r
        return out

    def evaluate(a, bprime):
        bhat = combine(bprime, cols)
        gm = 0
        for gi in range(len(F)):
            if parity(F[gi] & a) and parity(G[gi] & bprime):
                gm |= 1 << gi
        return recombine(tree_vec(a, bhat), gm), bhat

    pairs = ([(i, j) for i in range(lam) for j in range(nb)]
             if not quick else
             [(random.randrange(lam), random.randrange(nb))
              for _ in range(4000)])
    for i, j in pairs:
        a, bhat = 1 << i, cols[j]
        gm = 0
        for gi in range(len(F)):
            if ((F[gi] >> i) & 1) and ((G[gi] >> j) & 1):
                gm |= 1 << gi
        if recombine(tree_vec(a, bhat), gm) != pmod(pmul(a, bhat), p):
            raise AssertionError(f"basis check failed at (i,j)=({i},{j})")
    for _ in range(n_random):
        a = random.getrandbits(lam)
        bprime = random.getrandbits(nb)
        out, bhat = evaluate(a, bprime)
        if out != pmod(pmul(a, bhat), p):
            raise AssertionError("random dense check failed")
    return len(pairs) + n_random

def verify_wcrt(Wcrt, tree_moduli, lam, nb):
    """Standalone check that Wcrt is the CRT lift of the tree moduli, using
    only the stored table and the moduli (no crt_lift_cols).  Column j (the
    j-th residue bit, belonging to modulus M_i at local position b) must be
    the polynomial of degree < nb that is x^b mod M_i and 0 modulo every
    other tree modulus -- the defining property of the CRT idempotent basis,
    which pins Wcrt uniquely and guarantees it preserves residues.

    Checks raise AssertionError (never a bare `assert`) so they survive
    `python -O`, which strips assert statements."""
    if len(Wcrt) != lam:
        raise AssertionError("W_crt must have lambda rows")
    cols = [sum(((Wcrt[r] >> j) & 1) << r for r in range(lam))
            for j in range(nb)]
    off = 0
    for t, Mt in enumerate(tree_moduli):
        for b in range(pdeg(Mt)):
            c = cols[off + b]
            if pdeg(c) >= nb:
                raise AssertionError("W_crt column exceeds degree n_delta_bits")
            for tp, Mtp in enumerate(tree_moduli):
                if pmod(c, Mtp) != ((1 << b) if tp == t else 0):
                    raise AssertionError(
                        "stored W_crt is not the CRT lift of the tree moduli")
        off += pdeg(Mt)
    if off != nb:
        raise AssertionError("tree moduli degrees do not sum to n_delta_bits")

# ----------------------------------------------------------------------
# Self-tests of the building blocks
# ----------------------------------------------------------------------

def selftest():
    P = pascal_rows(7)
    Id = [combine(P[i], P) for i in range(7)]
    assert Id == [1 << i for i in range(7)], "Pascal not involutive"
    rng = random.Random(1)
    for n in range(1, 10):
        A = full_alg(n)
        assert len(A.gates) == full_cost(n)[0]
        S = short_alg(n)
        assert len(S.gates) == short_cost(n)[0]
        for i in range(n):
            for j in range(n):
                a, b = 1 << i, 1 << j
                assert eval_alg(A, a, b) == pmul(a, b), (n, i, j, "full")
                exp = pmul(a, b) & ((1 << n) - 1)
                assert eval_alg(S, a, b) == exp, (n, i, j, "short")
        for _ in range(50):
            a = rng.getrandbits(n)
            b = rng.getrandbits(n)
            assert eval_alg(A, a, b) == pmul(a, b)
            assert eval_alg(S, a, b) == pmul(a, b) & ((1 << n) - 1)
    for d in (6, 8):
        for q in irreducibles_of_degree(d)[:4]:
            alg, _ = tower_place_alg(q, d + 3, d + 2)  # asymmetric widths
            assert len(alg.gates) == TOWER_FIELD[d]
            for _ in range(200):
                a = rng.getrandbits(d + 3)
                b = rng.getrandbits(d + 2)
                assert eval_alg(alg, a, b) == pmod(pmul(a, b), q), (d, q)
    print("selftest OK: full/short algorithms n=1..9 match schoolbook; "
          "gate counts match the cost DP; tower deg-6/8 places (15/24 "
          "gates) match direct field arithmetic")
    print("  full_cost:", {n: full_cost(n)[0] for n in range(1, 11)})
    print("  short_cost:", {n: short_cost(n)[0] for n in range(1, 11)})

# ----------------------------------------------------------------------
# Output
# ----------------------------------------------------------------------

def rows_to_hex(rows):
    return [format(r, 'x') for r in rows]

def emit_json(path, meta, F, G, Wt, Wg, tree_moduli, lam):
    obj = dict(meta)
    obj['F'] = rows_to_hex(F)
    obj['G'] = rows_to_hex(G)
    obj['W_tree'] = rows_to_hex(Wt)
    obj['W_gate'] = rows_to_hex(Wg)
    obj['W_crt'] = rows_to_hex(wcrt_rows(tree_moduli, lam))
    obj['tree_reduction_rows'] = [rows_to_hex(reduction_rows(M, lam))
                                  for M in tree_moduli]
    with open(path, 'w') as f:
        json.dump(obj, f, indent=1)

# ---------------------------------------------------------------------------
# The `semantics` block embedded in every generated JSON file.  Notation
# follows the spec section "VOLE Commitment Masks with CRT"
# (commit-crt-mask.tex): Delta' / Delta, u_low / u_hi, res_tau, the gate index
# e, the conversions ToPoly / ToBits, and 0-based tree indices M_0..M_{tau-1}.
# Kept module-level so a consumer can re-emit it without re-running a search.
# ---------------------------------------------------------------------------
SEMANTICS = {
    'encoding':
        'every hex string encodes an integer whose bit i is component i of '
        'an F_2 vector, and a polynomial of degree < k is identified with '
        "its coefficient vector, bit i = coeff of x^i (the spec's ToPoly / "
        "ToBits); <r,v> below is the F_2 inner product parity(r AND v), "
        'written in the spec as the row-times-column product r * v',
    'inputs':
        "u in F_2^lambda; Dp (the spec's Delta') in F_2^n_delta_bits is the "
        'vector of tree residue bits of Delta, concatenated tree by tree; '
        'D (the spec\'s Delta) = W_crt * Dp in F_2^lambda is the CRT lift, '
        'the unique representative of degree < n_delta_bits',
    'tree_residue_vector':
        "r(u,Dp) in F_2^n_tree_bits (the spec's res_tau(u,Delta)): for each "
        'M_i in tree_moduli_hex (i = 0..tau-1, in order) compute s_i = '
        '(u mod M_i)*(D mod M_i) mod M_i in F_2[x]; bit b of s_i sits at '
        'offset deg(M_0)+..+deg(M_(i-1)) + b; note D mod M_i is exactly the '
        'slice of Dp belonging to M_i',
    'gate_vector':
        'g(u,Dp) in F_2^n_gates, indexed by the gate index e: '
        'g[e] = <F[e],u> * <G[e],Dp>, i.e. the componentwise product '
        '(F u) * (G Dp) of the two linear maps f_e, g_e',
    'identity':
        'for all u in F_2^lambda, Dp in F_2^n_delta_bits and all '
        'k < lambda: coefficient k of (u*D mod modulus_hex) = '
        '<W_tree[k], r(u,Dp)> XOR <W_gate[k], g(u,Dp)>; in the matrix form '
        'used by the spec, u*D = W_tree * res_tau(u,D) + W_gate * '
        '(F u * G Dp)',
    'verification':
        'both sides are bilinear in (u,Dp), so checking the identity on the '
        'lambda*(lambda-wgrind) basis pairs (u,Dp)=(x^i,e_j) is a complete '
        'proof; --verify does exactly this from this file alone, forming D '
        'by multiplying Dp with the stored W_crt, and separately checks '
        'W_crt is the CRT lift of the moduli (column j is x^b mod M_i, 0 '
        'mod the other M_i) and M_tree = prod(M_i)',
    'aux':
        'M_tree_hex = prod(M_i) (degree lambda - wgrind) is not part of the '
        'identity: the prover assembles a full-field mask as '
        'u = W_crt * u_low + M_tree(x) * ToPoly(u_hi) from the tree residue '
        'bits u_low and wgrind extra secret bits u_hi (F_2[x] '
        'multiplication by M_tree). tree_reduction_rows[i] are the rows of '
        'the linear map u -> u mod M_i over the lambda input bits; '
        'convenience data only. All are recomputable from tree_moduli_hex',
}


def emit_c(path, name, F, G, Wt, Wg, tree_moduli, lam, wgrind, ntree):
    """Emit a C header/source pair for one table set and return the .c path.

    The header (`path`, a .h) declares every table `extern const`, defines the
    parameter and per-table row-width macros, and states compile-time
    consistency assertions; the source (same stem, .c) holds the single
    definition of each table.  Splitting declaration from definition keeps
    exactly one copy of the (large) tables in the linked binary regardless of
    how many translation units include the header -- a `static const` table in
    a header is copied into every includer and trips -Wunused-const-variable
    in any TU that does not touch it -- and lets a shared apply routine bind
    the tables by name.
    """
    Wcrt = wcrt_rows(tree_moduli, lam)
    m_tree = mtree_poly(tree_moduli)
    ng = len(F)
    nb = lam - wgrind
    tau = len(tree_moduli)
    assert all(pdeg(M) < 64 for M in tree_moduli), \
        "tree modulus degree >= 64 does not fit one uint64 word"

    def words_of(width):
        return (width + 63) // 64

    w_F, w_G = words_of(lam), words_of(nb)
    w_WT, w_WG, w_WC = words_of(ntree), words_of(ng), words_of(nb)
    w_MT = words_of(m_tree.bit_length())

    # C identifier base: prefix with FAEST_ and upper-case so preset names
    # that begin with a digit (e.g. 128s) become valid identifiers.
    pfx = "FAEST_" + name.upper().replace('-', '_')
    guard = pfx + "_TABLES_H"
    src_path = path[:-2] + '.c' if path.endswith('.h') else path + '.c'
    header_name = os.path.basename(path)

    # (suffix, rows, words-per-row, rows-dim macro, words-dim macro): the
    # rows/words macros dimension the array so the extern declaration and the
    # definition are written with identical, self-documenting bounds.
    tables = [
        ('F',      F,    w_F,  f"{pfx}_NGATES", f"{pfx}_F_WORDS"),
        ('G',      G,    w_G,  f"{pfx}_NGATES", f"{pfx}_G_WORDS"),
        ('W_TREE', Wt,   w_WT, f"{pfx}_LAMBDA", f"{pfx}_W_TREE_WORDS"),
        ('W_GATE', Wg,   w_WG, f"{pfx}_LAMBDA", f"{pfx}_W_GATE_WORDS"),
        ('W_CRT',  Wcrt, w_WC, f"{pfx}_LAMBDA", f"{pfx}_W_CRT_WORDS"),
    ]

    def row_literal(r, words):
        ws = ', '.join(f"0x{(r >> (64 * w)) & ((1 << 64) - 1):016x}ULL"
                       for w in range(words))
        return f"{{ {ws} }}"

    # -------- header (.h): declarations, macros, static assertions --------
    with open(path, 'w') as f:
        f.write(f"/* generated by vole_mult_tables.py: {name}\n"
                " *\n"
                " * Bilinear VOLE-multiplication tables; see the companion\n"
                f" * tables_{name}.json for the full bilinear identity and\n"
                " * semantics.  Definitions live in the companion .c file.\n"
                " *\n"
                " * Encoding: each table row is an F_2 bit-vector packed\n"
                " * little-endian into uint64 words -- bit i sits in word i/64\n"
                " * at bit position i%64, and word 0 holds the low bits.\n"
                " */\n")
        f.write(f"#ifndef {guard}\n#define {guard}\n\n")
        f.write("#include <stdint.h>\n\n")
        # Portable file-scope static assert (C11 _Static_assert / C++11
        # static_assert, with a negative-array-size fallback for pre-C11).
        f.write(
            "#ifndef FAEST_STATIC_ASSERT\n"
            "#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L\n"
            "#define FAEST_STATIC_ASSERT(c, m) _Static_assert(c, m)\n"
            "#elif defined(__cplusplus) && __cplusplus >= 201103L\n"
            "#define FAEST_STATIC_ASSERT(c, m) static_assert(c, m)\n"
            "#else\n"
            "#define FAEST_SA_CONCAT_(a, b) a##b\n"
            "#define FAEST_SA_CONCAT(a, b) FAEST_SA_CONCAT_(a, b)\n"
            "#define FAEST_STATIC_ASSERT(c, m) \\\n"
            "  typedef char FAEST_SA_CONCAT(faest_static_assert_, __LINE__)"
            "[(c) ? 1 : -1]\n"
            "#endif\n"
            "#endif\n\n")
        # Parameters.
        f.write(f"#define {pfx}_LAMBDA {lam}\n")
        f.write(f"#define {pfx}_NGATES {ng}\n")
        f.write(f"#define {pfx}_WGRIND {wgrind}\n")
        f.write(f"#define {pfx}_NDELTA_BITS {nb}\n")
        f.write(f"#define {pfx}_NTREE_BITS {ntree}\n")
        f.write(f"#define {pfx}_TAU {tau}\n\n")
        # Per-table row widths (uint64 words), baked into the array types.
        f.write(f"#define {pfx}_F_WORDS {w_F}\n")
        f.write(f"#define {pfx}_G_WORDS {w_G}\n")
        f.write(f"#define {pfx}_W_TREE_WORDS {w_WT}\n")
        f.write(f"#define {pfx}_W_GATE_WORDS {w_WG}\n")
        f.write(f"#define {pfx}_W_CRT_WORDS {w_WC}\n")
        f.write(f"#define {pfx}_M_TREE_WORDS {w_MT}\n\n")
        # Declarations (definitions are in the .c).
        for suf, _rows, _w, rmac, wmac in tables:
            f.write(f"extern const uint64_t {pfx}_{suf}[{rmac}][{wmac}];\n")
        f.write(f"extern const uint64_t {pfx}_TREE_MODULI[{pfx}_TAU];\n")
        f.write(f"extern const uint64_t {pfx}_M_TREE[{pfx}_M_TREE_WORDS];\n\n")
        # Consistency assertions: the row-width macros are compile-time
        # constants baked into the array types, so a consumer's per-row loop
        # bound is fixed at compile time.  Pinning each width to the parameter
        # it derives from turns a hand-edit or partial regeneration that
        # desyncs them into a build failure instead of a silently truncated or
        # over-long row read at run time.
        f.write("/* fail to compile if a width macro is inconsistent with the "
                "relevant parameter */\n")

        def sa(cond, msg):
            f.write(f"FAEST_STATIC_ASSERT({cond},\n"
                    f"                    \"{pfx}: {msg}\");\n")

        sa(f"{pfx}_NDELTA_BITS == {pfx}_LAMBDA - {pfx}_WGRIND",
           "NDELTA_BITS must equal LAMBDA - WGRIND")
        sa(f"{pfx}_NTREE_BITS == {pfx}_NDELTA_BITS",
           "tree residue width must equal NDELTA_BITS")
        sa(f"{pfx}_F_WORDS == ({pfx}_LAMBDA + 63) / 64",
           "F_WORDS desynced from LAMBDA")
        sa(f"{pfx}_G_WORDS == ({pfx}_NDELTA_BITS + 63) / 64",
           "G_WORDS desynced from NDELTA_BITS")
        sa(f"{pfx}_W_TREE_WORDS == ({pfx}_NTREE_BITS + 63) / 64",
           "W_TREE_WORDS desynced from NTREE_BITS")
        sa(f"{pfx}_W_GATE_WORDS == ({pfx}_NGATES + 63) / 64",
           "W_GATE_WORDS desynced from NGATES")
        sa(f"{pfx}_W_CRT_WORDS == ({pfx}_NDELTA_BITS + 63) / 64",
           "W_CRT_WORDS desynced from NDELTA_BITS")
        f.write(f"\n#endif /* {guard} */\n")

    # -------- source (.c): the single definition of each table --------
    with open(src_path, 'w') as f:
        f.write(f"/* generated by vole_mult_tables.py: {name} */\n")
        f.write(f"#include \"{header_name}\"\n\n")
        for suf, rows, words, rmac, wmac in tables:
            f.write(f"const uint64_t {pfx}_{suf}[{rmac}][{wmac}] = {{\n")
            for r in rows:
                f.write(f"  {row_literal(r, words)},\n")
            f.write("};\n\n")
        # Tree moduli M_i and their product M_tree (used by the prover to
        # assemble a full-field mask u = W_crt u_low + M_tree(x) u_hi(x)).
        moduli = ', '.join(f"0x{M:x}ULL" for M in tree_moduli)
        f.write(f"const uint64_t {pfx}_TREE_MODULI[{pfx}_TAU] = "
                f"{{ {moduli} }};\n\n")
        mt = ', '.join(f"0x{(m_tree >> (64 * w)) & ((1 << 64) - 1):016x}ULL"
                       for w in range(w_MT))
        f.write(f"const uint64_t {pfx}_M_TREE[{pfx}_M_TREE_WORDS] = "
                f"{{ {mt} }};\n")
    return src_path

# ----------------------------------------------------------------------
# Main
# ----------------------------------------------------------------------

# (lambda, tau, w_grind) per FAEST round-3 parameter set; the tree degree
# portfolio follows from these via faest_tree_spec below.  Note the grinding
# parameter is what makes FAEST-256f feasible: its trees are 24 x deg-8 +
# 8 x deg-7 (sum 248 = lambda - wgrind), comfortably within the 30 monic
# irreducibles of degree 8 over F_2.
PRESETS = {
    '128s': (128, 11, 7),
    '128f': (128, 17, 8),
    '192s': (192, 16, 12),
    '192f': (192, 24, 8),
    '256s': (256, 22, 6),
    '256f': (256, 33, 8),
    # FAEST-EM variants share (lambda, tau, wgrind) with the AES variants
    # above -- and hence the same tables -- except at lambda = 192, where both
    # EM variants have their own (tau, wgrind):
    '192s_em': (192, 16, 8),
    '192f_em': (192, 25, 8),
}

def faest_tree_spec(lam, tau, wgrind):
    """FAEST small-VOLE depths: tau_1 = (lam-w) mod tau trees of depth
    k = floor((lam-w)/tau) + 1, the rest of depth k - 1.  (The spec writes
    d_i for the depth of tree i, so d_i = k for i < tau_1 and k - 1
    otherwise; do not confuse that per-tree d_i with the two size classes
    named here.)  Note tau_1 = 0 is possible -- it holds for 128s, where
    tau divides lam - w -- and then every tree has depth k - 1 and no
    modulus of degree k is emitted.
    Returns [(degree, count), ...], larger degree first."""
    n = lam - wgrind
    d1 = n // tau + 1
    tau1 = n % tau
    spec = []
    if tau1:
        spec.append((d1, tau1))
    spec.append((d1 - 1, tau - tau1))
    return spec

def parse_trees(s):
    out = []
    for part in s.split(','):
        d, c = part.lower().split('x')
        out.append((int(d), int(c)))
    return out

def run_set(name, lam, wgrind, tree_spec, args, out_path=None, c_path=None):
    """Search, build, exhaustively verify, and optionally write one table
    set.  Returns (n_tree_bits, n_gates)."""
    tree_sum = sum(d * c for d, c in tree_spec)
    tau = sum(c for _, c in tree_spec)
    assert tree_sum == lam - wgrind, \
        f"tree degrees sum to {tree_sum}, expected lambda - wgrind = {lam - wgrind}"
    N2 = lam + (lam - wgrind) - 1   # product coefficients (deg u < lam,
                                    # deg Delta < lam - wgrind)
    D = N2 - tree_sum               # = lam - 1, independent of wgrind
    print(f"[{name}] lambda={lam} tau={tau} wgrind={wgrind} tree degrees="
          f"{'+'.join(f'{c}x{d}' for d, c in tree_spec)} "
          f"(sum {tree_sum}), degree deficit D = {D}")

    # choose tree moduli deterministically, build exclusion pools
    tree_moduli = []
    for d, c in tree_spec:
        pool = irreducibles_of_degree(d)
        if c > len(pool):
            raise SystemExit(f"only {len(pool)} irreducibles of degree {d} "
                             f"over F_2; cannot pick {c} tree moduli")
        tree_moduli += list(pool[:c])
    treeset = set(tree_moduli)
    pools = {d: [q for q in irreducibles_of_degree(d) if q not in treeset]
             for d in range(2, args.maxd + 1)}

    p = find_modulus(lam)
    print(f"  field modulus p = 0x{p:x}")

    res = crt_search(D, args.maxe, args.maxd, pools)
    assert res is not None, "portfolio search failed; raise --maxd/--maxe"
    cost_con, picks = res
    print(f"  portfolio found: {cost_con} gates")

    F, G, Wt, Wg, ntree, report = build_tables(lam, wgrind, p, tree_moduli,
                                               picks)
    n_raw = len(F)
    F, G, Wg = prune(F, G, Wg)
    ng = len(F)
    print(f"  built {n_raw} gates, {ng} after pruning "
          f"(zero/duplicate/unused removal)")

    def pk_str(pk):
        kind, poly, e = pk
        if kind == 'inf':
            return f"inf^{e}"
        if kind == 'lin':
            return f"({'x' if poly == 2 else 'x+1'})^{e}"
        return f"(0x{poly:x})^{e}" if e > 1 else f"0x{poly:x}"
    agg = {}
    for pk, g in report:
        kind, poly, e = pk
        d = e if kind in ('inf', 'lin') else pdeg(poly) * e
        key = (kind if kind != 'irr' else f"irr deg {pdeg(poly)}", e)
        agg.setdefault(key, [0, 0, 0])
        agg[key][0] += 1
        agg[key][1] += d
        agg[key][2] += g
    print("  extra places (type, mult): count, total degree, gates")
    for key, (cnt, dtot, gtot) in sorted(agg.items(), key=lambda kv: kv[0][0]):
        print(f"    {key[0]:<10} e={key[1]}: {cnt:>3} places, deg {dtot:>3}, "
              f"gates {gtot:>3}")

    Wcrt = wcrt_rows(tree_moduli, lam)
    verify_wcrt(Wcrt, tree_moduli, lam, lam - wgrind)
    nchecks = verify_tables(lam, wgrind, p, tree_moduli, F, G, Wt, Wg, Wcrt,
                            quick=args.quick)
    print(f"  verified on {nchecks} pairs "
          f"({'sampled' if args.quick else 'all basis pairs -> complete proof'})")

    print(f"  => gate correction bits per F_2^{lam} mask: {ng} "
          f"(the {ntree} tree residue bits come free from the GGM trees)")

    meta = {
        'format_version': 1,
        'name': name,
        'lambda': lam,
        'tau': tau,
        'wgrind': wgrind,
        'n_delta_bits': lam - wgrind,
        'modulus_hex': format(p, 'x'),
        'tree_moduli_hex': [format(M, 'x') for M in tree_moduli],
        'M_tree_hex': format(mtree_poly(tree_moduli), 'x'),
        'portfolio': [{'place': pk_str(pk), 'gates': g} for pk, g in report],
        'n_gates': ng,
        'n_tree_bits': ntree,
        'gate_bits_per_mask': ng,
        'semantics': SEMANTICS,
    }
    if out_path:
        emit_json(out_path, meta, F, G, Wt, Wg, tree_moduli, lam)
        print(f"  wrote {out_path}")
    if c_path:
        src_path = emit_c(c_path, name, F, G, Wt, Wg, tree_moduli, lam,
                          wgrind, ntree)
        print(f"  wrote {c_path} and {src_path}")
    return ntree, ng

def do_verify(path, quick):
    with open(path) as f:
        obj = json.load(f)
    lam = obj['lambda']
    wgrind = obj['wgrind']
    p = int(obj['modulus_hex'], 16)
    trees = [int(h, 16) for h in obj['tree_moduli_hex']]
    F = [int(h, 16) for h in obj['F']]
    G = [int(h, 16) for h in obj['G']]
    Wt = [int(h, 16) for h in obj['W_tree']]
    Wg = [int(h, 16) for h in obj['W_gate']]
    Wcrt = [int(h, 16) for h in obj['W_crt']]
    # Check the stored W_crt is the CRT lift and M_tree the product of the
    # moduli, using only the stored tables (no construction machinery); then
    # prove the bilinear identity, multiplying Delta' by that same W_crt.
    verify_wcrt(Wcrt, trees, lam, lam - wgrind)
    if int(obj['M_tree_hex'], 16) != mtree_poly(trees):
        raise AssertionError(
            "stored M_tree does not match the product of the tree moduli")
    n = verify_tables(lam, wgrind, p, trees, F, G, Wt, Wg, Wcrt, quick=quick)
    print(f"verified {path}: {n} checks passed "
          f"({len(F)} gates, {sum(pdeg(M) for M in trees)} tree bits, "
          f"wgrind={wgrind}; W_crt is the CRT lift, M_tree = prod(M_i))")

def main():
    ap = argparse.ArgumentParser(
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description=(
            "Search + frozen-table generation for CRT-based F_{2^lambda} "
            "VOLE multiplication.\n"
            "Every generated table set is proven correct on all lambda^2 "
            "basis pairs before it is written."),
        epilog=(
            "typical usage:\n"
            "  %(prog)s --all\n"
            "      run the self-tests, then generate, verify, and write\n"
            "      tables_<preset>.json and tables_<preset>.h for every\n"
            "      parameter set (" + ", ".join(sorted(PRESETS)) + ")\n"
            "  %(prog)s --all --outdir tables/\n"
            "      same, into a chosen directory\n"
            "  %(prog)s --preset 128f --out t.json --emit-c t.h\n"
            "      generate a single parameter set\n"
            "  %(prog)s --lam 128 --wgrind 8 --trees 8x8,7x8 --out t.json\n"
            "      custom tree portfolio (degree x count, summing to\n"
            "      lambda - wgrind)\n"
            "  %(prog)s --verify t.json\n"
            "      independently re-verify a table file from scratch\n"
            "  %(prog)s --selftest\n"
            "      check the formula building blocks against schoolbook\n"))
    act = ap.add_argument_group('actions (pick one)')
    act.add_argument('--all', action='store_true',
                     help='selftest + generate/verify/write every preset')
    act.add_argument('--preset', choices=sorted(PRESETS),
                     help='generate one named parameter set')
    act.add_argument('--lam', type=int, metavar='LAMBDA',
                     help='generate a custom set (with --trees)')
    act.add_argument('--verify', type=str, metavar='FILE.json',
                     help='re-verify an existing table file')
    act.add_argument('--selftest', action='store_true',
                     help='verify the formula building blocks only')
    opt = ap.add_argument_group('options')
    opt.add_argument('--trees', type=str, metavar='DxC,DxC,...',
                     help='tree degree portfolio for --lam, e.g. "8x8,7x8" '
                          '(degrees summing to lambda - wgrind)')
    opt.add_argument('--wgrind', type=int, default=0,
                     help='grinding parameter for --lam (default 0)')
    opt.add_argument('--out', type=str, metavar='FILE.json',
                     help='output table file (single-set modes)')
    opt.add_argument('--emit-c', type=str, metavar='FILE.h',
                     help='also emit a C header (single-set modes)')
    opt.add_argument('--outdir', type=str, default='.',
                     help='output directory for --all (default: .)')
    opt.add_argument('--maxd', type=int, default=10,
                     help='max degree of extra irreducible places (default '
                          '10; matters when the tree moduli deplete the '
                          'cheap low-degree pools, e.g. 256f)')
    opt.add_argument('--maxe', type=int, default=10,
                     help='max multiplicity of linear/infinity places '
                          '(default 10)')
    opt.add_argument('--quick', action='store_true',
                     help='sampled instead of exhaustive verification '
                          '(exhaustive takes ~1-2 min at lambda=256)')
    opt.add_argument('--seed', type=int, default=0,
                     help='RNG seed for the random dense checks '
                          '(basis-pair checks are deterministic)')
    args = ap.parse_args()
    random.seed(args.seed)

    if not (args.all or args.preset or args.lam or args.verify
            or args.selftest):
        ap.print_help()
        return

    if args.selftest:
        selftest()
        return

    if args.verify:
        do_verify(args.verify, args.quick)
        return

    if args.all:
        selftest()
        print()
        os.makedirs(args.outdir, exist_ok=True)
        summary = []
        for name in sorted(PRESETS):
            lam, tau, wgrind = PRESETS[name]
            tree_spec = faest_tree_spec(lam, tau, wgrind)
            out = os.path.join(args.outdir, f"tables_{name}.json")
            ch = os.path.join(args.outdir, f"tables_{name}.h")
            ntree, ng = run_set(name, lam, wgrind, tree_spec, args, out, ch)
            summary.append((name, lam, ntree, ng))
            print()
        print("summary (all table sets verified on all basis pairs):")
        for name, lam, ntree, ng in summary:
            print(f"  {name:>4}: lambda={lam:<3} gates={ng:<4} "
                  f"(= correction bits per mask)")
        return

    if args.preset:
        lam, tau, wgrind = PRESETS[args.preset]
        tree_spec = faest_tree_spec(lam, tau, wgrind)
        name = args.preset
    else:
        if not args.trees:
            ap.error("--lam requires --trees (e.g. --trees 8x8,7x8)")
        lam, wgrind = args.lam, args.wgrind
        tree_spec = parse_trees(args.trees)
        name = f"custom{lam}"
    run_set(name, lam, wgrind, tree_spec, args, args.out, args.emit_c)

if __name__ == '__main__':
    main()
