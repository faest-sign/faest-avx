from math import ceil, floor, log
import sys

def faest_vole_commit(tau, sec, ell, B):
    """
    Commitment to the PPRF instances and correction values
     - ell: length of initial random-VOLE instance
    """
    # send one hash for all tau PPRFs
    hash_len = 2*sec
    #correction_values = (ell + tau) * (tau - 1) * logp
    correction_values = ell * (tau - 1)

    #if commit_to_corrections: # NB only possible with 2-out-of-2 sharing
    #    correction_values /= 2
    #    d_elements /= 2

    # VOLE consistency check
    check_u = sec + B
    check_v = hash_len # only send the hash

    # check_v is only hashed, so we don't need to send it. The same is true for the hash of the
    # leaves.
    total = correction_values + check_u
    return total/8

def faest_aes_proof(logp, sec, witness_len):
    commit_to_witness = logp * witness_len
    check_values = 1*sec # There's a second check value, but it's only hashed, so its free.
    return (commit_to_witness + check_values) / 8

def faest_challenge_decom(sec, tau):
    """ Response to the last challenge """
    k0 = ceil(sec/tau)
    k1 = floor(sec/tau)
    tau0 = sec % tau
    tau1 = tau - tau0
    return ((k0*tau0 + k1*tau1) * sec + 2*sec*tau)/8
    #return (logq * sec + 2*sec) * tau / 8

# number of s-box input bytes, excluding the witness
sboxes = {
    128: {'e': 160 - 16, 'c': 200 - 16},
    192: {'e': 288 - 24, 'c': 416 - 2*16},
    256: {'e': 448 - 32, 'c': 500 - 2*16}
}

def faest_sizes(num_sboxes, tau, sec):
    logp = 1
    B = 16 # extra padding bits for VOLE universal hash
    #witness_length = 8 * num_sboxes
    witness_length = sec + 8 * num_sboxes
    #witness_length = get_witness_length(sec, EM)

    k0 = ceil(sec/tau)
    k1 = floor(sec/tau)
    commit_cost = faest_vole_commit(tau, sec, witness_length + 2*sec + B, B) + faest_aes_proof(logp, sec, witness_length)
    open_cost = faest_challenge_decom(sec, tau)
    chal3_cost = sec / 8
    iv_cost = 128 / 8
    sig_size = commit_cost + open_cost + chal3_cost + iv_cost

    print(round(sig_size))

if __name__ == '__main__':
    sec = int(sys.argv[1])
    assert(sec in [128, 192, 256])

    tau = int(sys.argv[2])

    owf = sys.argv[3]
    assert(owf in ["c", "e"])

    num_sboxes = sboxes[sec][owf]
    faest_sizes(sec=sec, num_sboxes=num_sboxes, tau=tau)
