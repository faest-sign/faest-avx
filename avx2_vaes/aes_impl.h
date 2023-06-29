#ifndef AES_IMPL_H
#define AES_IMPL_H

#include <immintrin.h>
#include <wmmintrin.h>

#define AES_VECTOR_WIDTH_SHIFT 1

// TODO: Stuff in common with avx2

// Want to do each pair of AESes from adjacent columns, rather than the same column, so that they
// can use two adjacent keys.

#endif
