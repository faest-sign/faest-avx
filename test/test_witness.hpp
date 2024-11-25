#ifndef TEST_EXT_WITNESS_HPP
#define TEST_EXT_WITNESS_HPP

#include <array>
#include <cstdint>

extern const std::array<uint8_t, 16> AES_ECB_128_KEY;
extern const std::array<uint8_t, 16> AES_ECB_128_INPUT;
extern const std::array<uint8_t, 16> AES_ECB_128_OUTPUT;
extern const std::array<uint8_t, 200> AES_ECB_128_EXTENDED_WITNESS;

extern const std::array<uint8_t, 24> AES_ECB_192_KEY;
extern const std::array<uint8_t, 32> AES_ECB_192_INPUT;
extern const std::array<uint8_t, 32> AES_ECB_192_OUTPUT;
extern const std::array<uint8_t, 408> AES_ECB_192_EXTENDED_WITNESS;

extern const std::array<uint8_t, 32> AES_ECB_256_KEY;
extern const std::array<uint8_t, 32> AES_ECB_256_INPUT;
extern const std::array<uint8_t, 32> AES_ECB_256_OUTPUT;
extern const std::array<uint8_t, 500> AES_ECB_256_EXTENDED_WITNESS;

extern const std::array<uint8_t, 16> RIJNDAEL_EM_128_KEY;
extern const std::array<uint8_t, 16> RIJNDAEL_EM_128_INPUT;
extern const std::array<uint8_t, 16> RIJNDAEL_EM_128_OUTPUT;
extern const std::array<uint8_t, 160> RIJNDAEL_EM_128_EXTENDED_WITNESS;

extern const std::array<uint8_t, 24> RIJNDAEL_EM_192_KEY;
extern const std::array<uint8_t, 24> RIJNDAEL_EM_192_INPUT;
extern const std::array<uint8_t, 24> RIJNDAEL_EM_192_OUTPUT;
extern const std::array<uint8_t, 288> RIJNDAEL_EM_192_EXTENDED_WITNESS;

extern const std::array<uint8_t, 32> RIJNDAEL_EM_256_KEY;
extern const std::array<uint8_t, 32> RIJNDAEL_EM_256_INPUT;
extern const std::array<uint8_t, 32> RIJNDAEL_EM_256_OUTPUT;
extern const std::array<uint8_t, 448> RIJNDAEL_EM_256_EXTENDED_WITNESS;

#endif // TEST_EXT_WITNESS_HPP
