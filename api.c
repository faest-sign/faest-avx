#include <assert.h>
#include <string.h>

#include "api.h"
#include "faest.h"
#include "randomness.h"

static_assert(CRYPTO_PUBLICKEYBYTES == FAEST_PUBLIC_KEY_BYTES, "");
static_assert(CRYPTO_SECRETKEYBYTES == FAEST_SECRET_KEY_BYTES, "");
static_assert(CRYPTO_BYTES == FAEST_SIGNATURE_BYTES, "");

// Sample a random key pair (pk, sk). pk and sk which must have length CRYPTO_PUBLICKEYBYTES and
// CRYPTO_SECRETKEYBYTES, respectively.
int crypto_sign_keypair(unsigned char* pk, unsigned char* sk)
{
	do
	{
		rand_bytes(sk, FAEST_SECRET_KEY_BYTES);
	} while (!faest_pubkey(pk, sk));
	return 0;
}

// Generate a signed message sm = m || signature(sk, m). m must have length mlen. sm must have
// length mlen + CRYPTO_BYTES, and this length will be written into *smlen. sk must have length
// CRYPTO_SECRETKEYBYTES.
int crypto_sign(
	unsigned char *sm, unsigned long long *smlen,
	const unsigned char *m, unsigned long long mlen,
	const unsigned char *sk)
{
	*smlen = mlen + FAEST_SIGNATURE_BYTES;
	memmove(sm, m, mlen);

	uint8_t random_seed[SECURITY_PARAM / 8];
	rand_bytes(random_seed, sizeof(random_seed));
	faest_sign(sm + mlen, sm, mlen, sk, random_seed, sizeof(random_seed));
	return 0;
}

// Verify a signed message sm of length smlen, which must be at least CRYPTO_BYTES. If the
// signature is correct, write the original message into m and set *mlen = smlen - CRYPTO_BYTES. If
// not, return -1. pk must have length CRYPTO_PUBLICKEYBYTES.
int crypto_sign_open(
	unsigned char *m, unsigned long long *mlen,
	const unsigned char *sm, unsigned long long smlen,
	const unsigned char *pk)
{
	unsigned long long m_length = smlen - FAEST_SIGNATURE_BYTES;
	if (!faest_verify(sm + m_length, sm, m_length, pk))
		return -1;

	*mlen = m_length;
	memmove(m, sm, m_length);
	return 0;
}
