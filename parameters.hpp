#ifndef PARAMETERS_HPP
#define PARAMETERS_HPP

#include <cstdint>
#include <type_traits>
#include <utility>

namespace faest
{

// Enum representing the security parameter
enum class secpar : std::size_t
{
    s128 = 128,
    s192 = 192,
    s256 = 256,
};

// Convert the security parameter to the corresponding number of bits
constexpr std::size_t secpar_to_bits(secpar s) { return std::to_underlying(s); }
// Convert the security parameter to the corresponding number of bytes
constexpr std::size_t secpar_to_bytes(secpar s) { return secpar_to_bits(s) / 8; }

namespace
{
constexpr unsigned int owf_algo_ecb = 0;
constexpr unsigned int owf_algo_em = 2;
constexpr unsigned int owf_algo_shift = 8;
constexpr unsigned int owf_flag_zero_sboxes = 0b0001;
constexpr unsigned int owf_flag_norm_proof = 0b0010;
constexpr unsigned int owf_flag_shrunk_keyspace = 0b0100;
constexpr unsigned int owf_flag_ctr_input = 0b1000;
} // namespace

// Enum of the supported one-way functions
enum class owf : unsigned int
{
    aes_ecb = owf_algo_ecb << owf_algo_shift,
    aes_em = owf_algo_em << owf_algo_shift,
    aes_ecb_with_zero_sboxes = aes_ecb | owf_flag_zero_sboxes,
    aes_em_with_zero_sboxes = aes_em | owf_flag_zero_sboxes,
    aes_ecb_with_zero_sboxes_and_norm_proof = aes_ecb_with_zero_sboxes | owf_flag_norm_proof,
    aes_em_with_zero_sboxes_and_norm_proof = aes_em_with_zero_sboxes | owf_flag_norm_proof,
    v1 = aes_ecb,
    v1_em = aes_em,
    v2 = aes_ecb | owf_flag_zero_sboxes | owf_flag_norm_proof | owf_flag_shrunk_keyspace |
         owf_flag_ctr_input,
    v2_em = aes_em | owf_flag_zero_sboxes | owf_flag_norm_proof | owf_flag_shrunk_keyspace,
};

constexpr bool is_owf_with_aes_ecb(owf o)
{
    return (std::to_underlying(o) >> owf_algo_shift) == owf_algo_ecb;
}

constexpr bool is_owf_with_aes_em(owf o)
{
    return (std::to_underlying(o) >> owf_algo_shift) == owf_algo_em;
}

constexpr bool is_owf_with_zero_sboxes(owf o)
{
    return std::to_underlying(o) & owf_flag_zero_sboxes;
}

constexpr bool is_owf_with_norm_proof(owf o) { return std::to_underlying(o) & owf_flag_norm_proof; }

constexpr bool is_owf_with_shrunk_keyspace(owf o)
{
    return std::to_underlying(o) & owf_flag_shrunk_keyspace;
}

constexpr bool is_owf_with_ctr_input(owf o) { return std::to_underlying(o) & owf_flag_ctr_input; }

// Enum of the supported PRGs
enum class prg
{
    aes_ctr,
    rijndael_fixed_key_ctr,
};

// Enum of the supported leaf hashes
enum class leaf_hash
{
    aes_ctr,
    aes_ctr_stat_bind,
    rijndael_fixed_key_ctr,
    rijndael_fixed_key_ctr_stat_bind,
    shake,
};

// Defined in prgs.hpp
template <secpar S> struct aes_ctr_prg;
template <secpar S> struct rijndael_fixed_key_ctr_prg;

// Defined in vector_com.hpp
template <typename PRG> struct prg_leaf_hash;
template <typename PRG, uint32_t MAX_TWEAKS> struct stat_binding_leaf_hash;
template <secpar S> struct shake_leaf_hash;

// Template to obtain the PRG type corresponding to a prg enum value
template <secpar S, prg PRG> struct prg_type;
template <secpar S, prg PRG> using prg_type_t = prg_type<S, PRG>::type;
template <secpar S> struct prg_type<S, prg::aes_ctr>
{
    using type = aes_ctr_prg<S>;
};
template <secpar S> struct prg_type<S, prg::rijndael_fixed_key_ctr>
{
    using type = rijndael_fixed_key_ctr_prg<S>;
};

// Template to obtain the leaf hash type corresponding to a leaf_hash enum value
template <secpar S, uint32_t MAX_TWEAKS, leaf_hash LH> struct leaf_hash_type;
template <secpar S, uint32_t MAX_TWEAKS, leaf_hash LH>
using leaf_hash_type_t = leaf_hash_type<S, MAX_TWEAKS, LH>::type;
template <secpar S, uint32_t MAX_TWEAKS>
struct leaf_hash_type<S, MAX_TWEAKS, leaf_hash::aes_ctr>
{
    using type = prg_leaf_hash<aes_ctr_prg<S>>;
};
template <secpar S, uint32_t MAX_TWEAKS>
struct leaf_hash_type<S, MAX_TWEAKS, leaf_hash::aes_ctr_stat_bind>
{
    using type = stat_binding_leaf_hash<aes_ctr_prg<S>, MAX_TWEAKS>;
};
template <secpar S, uint32_t MAX_TWEAKS>
struct leaf_hash_type<S, MAX_TWEAKS, leaf_hash::rijndael_fixed_key_ctr>
{
    using type = prg_leaf_hash<rijndael_fixed_key_ctr_prg<S>>;
};
template <secpar S, uint32_t MAX_TWEAKS>
struct leaf_hash_type<S, MAX_TWEAKS, leaf_hash::rijndael_fixed_key_ctr_stat_bind>
{
    using type = stat_binding_leaf_hash<rijndael_fixed_key_ctr_prg<S>, MAX_TWEAKS>;
};
template <secpar S, uint32_t MAX_TWEAKS>
struct leaf_hash_type<S, MAX_TWEAKS, leaf_hash::shake>
{
    using type = shake_leaf_hash<S>;
};

enum class bavc
{
    ggm_forest,
    one_tree,
};

template <secpar S, std::size_t TAU, std::size_t DELTA_BITS, prg TREE_PRG, leaf_hash LEAF_HASH,
          std::size_t VOLE_WIDTH_SHIFT>
struct ggm_forest_bavc;

template <secpar S, std::size_t TAU, std::size_t DELTA_BITS, prg TREE_PRG, leaf_hash LEAF_HASH,
          std::size_t VOLE_WIDTH_SHIFT, std::size_t OPENING_SEEDS_THRESHOLD>
struct one_tree_bavc;

template <secpar S, std::size_t TAU, std::size_t DELTA_BITS, prg TREE_PRG, leaf_hash LEAF_HASH,
          std::size_t VOLE_WIDTH_SHIFT, bavc BAVC, std::size_t BAVC_PARAM>
struct bavc_type;
template <secpar S, std::size_t TAU, std::size_t DELTA_BITS, prg TREE_PRG, leaf_hash LEAF_HASH,
          std::size_t VOLE_WIDTH_SHIFT, bavc BAVC, std::size_t BAVC_PARAM>
using bavc_type_t =
    bavc_type<S, TAU, DELTA_BITS, TREE_PRG, LEAF_HASH, VOLE_WIDTH_SHIFT, BAVC, BAVC_PARAM>::type;
template <secpar S, std::size_t TAU, std::size_t DELTA_BITS, prg TREE_PRG, leaf_hash LEAF_HASH,
          std::size_t VOLE_WIDTH_SHIFT>
struct bavc_type<S, TAU, DELTA_BITS, TREE_PRG, LEAF_HASH, VOLE_WIDTH_SHIFT, bavc::ggm_forest, 0>
{
    using type = ggm_forest_bavc<S, TAU, DELTA_BITS, TREE_PRG, LEAF_HASH, VOLE_WIDTH_SHIFT>;
};
template <secpar S, std::size_t TAU, std::size_t DELTA_BITS, prg TREE_PRG, leaf_hash LEAF_HASH,
          std::size_t VOLE_WIDTH_SHIFT, std::size_t BAVC_PARAM>
struct bavc_type<S, TAU, DELTA_BITS, TREE_PRG, LEAF_HASH, VOLE_WIDTH_SHIFT, bavc::one_tree,
                 BAVC_PARAM>
{
    using type =
        one_tree_bavc<S, TAU, DELTA_BITS, TREE_PRG, LEAF_HASH, VOLE_WIDTH_SHIFT, BAVC_PARAM>;
};

// Defined in constants.hpp
template <secpar S, owf P> struct OWF_CONSTANTS;
template <typename P> struct CONSTANTS;

// Template describing a particular instance of FAEST
//
// The parameters are
// - the security parameter
// - tau = number of bits per witness bit
//       = number of GGM trees
// - the one-way function to prove
// - the PRG used for the VOLEs
// - the PRG used for inner nodes in the GGM trees
// - the PRG used for leaf nodes of the GGM trees
// - number of zero bits in Delta
template <secpar S, std::size_t TAU, owf OWF, prg VOLE_PRG, prg TREE_PRG = prg::aes_ctr,
          leaf_hash LEAF_HASH = leaf_hash::shake, std::size_t ZERO_BITS_IN_DELTA = 0,
          std::pair<bavc, std::size_t> BAVC = {bavc::ggm_forest, 0}>
struct parameter_set
{
    // Values of the template parameters as constants
    constexpr static secpar secpar_v = S;
    constexpr static std::size_t tau_v = TAU;
    constexpr static owf owf_v = OWF;
    constexpr static prg vole_prg_v = VOLE_PRG;
    constexpr static prg tree_prg_v = TREE_PRG;
    constexpr static leaf_hash leaf_hash_v = LEAF_HASH;
    constexpr static std::size_t zero_bits_in_delta_v = ZERO_BITS_IN_DELTA;

    // Shorthands for the security parameter in bits and bytes
    constexpr static std::size_t secpar_bits = secpar_to_bits(S);
    constexpr static std::size_t secpar_bytes = secpar_to_bytes(S);

    // Size of Delta
    constexpr static std::size_t delta_bits_v = secpar_bits - zero_bits_in_delta_v;

    // Access to the implementation constants that depend on the parameters
    using CONSTS = CONSTANTS<
        parameter_set<S, TAU, OWF, VOLE_PRG, TREE_PRG, LEAF_HASH, ZERO_BITS_IN_DELTA, BAVC>>;
    // Access to the one-way function constants that depend on the parameters
    using OWF_CONSTS = OWF_CONSTANTS<S, OWF>;

    // The types of the selected PRGs
    using leaf_hash_t = leaf_hash_type_t<S, TAU, LEAF_HASH>;
    using tree_prg_t = prg_type_t<S, TREE_PRG>;
    using vole_prg_t = prg_type_t<S, VOLE_PRG>;

    // The batched all-but-one vector commitment
    using bavc_t = bavc_type_t<S, TAU, delta_bits_v, TREE_PRG, LEAF_HASH, CONSTS::VOLE_WIDTH_SHIFT,
                               BAVC.first, BAVC.second>;

    constexpr static bool use_grinding =
        (zero_bits_in_delta_v > 0) || !bavc_t::OPEN_ALWAYS_SUCCEEDS;
    constexpr static std::size_t grinding_counter_size = []
    {
        if (use_grinding)
            return 4;
        else
            return 0;
    }();
};

// The FAEST v1 instances
namespace v1
{
using faest_128_s = parameter_set<secpar::s128, 11, owf::aes_ecb, prg::aes_ctr>;
using faest_128_f = parameter_set<secpar::s128, 16, owf::aes_ecb, prg::aes_ctr>;
using faest_192_s = parameter_set<secpar::s192, 16, owf::aes_ecb, prg::aes_ctr>;
using faest_192_f = parameter_set<secpar::s192, 24, owf::aes_ecb, prg::aes_ctr>;
using faest_256_s = parameter_set<secpar::s256, 22, owf::aes_ecb, prg::aes_ctr>;
using faest_256_f = parameter_set<secpar::s256, 32, owf::aes_ecb, prg::aes_ctr>;

using faest_em_128_s = parameter_set<secpar::s128, 11, owf::aes_em, prg::aes_ctr>;
using faest_em_128_f = parameter_set<secpar::s128, 16, owf::aes_em, prg::aes_ctr>;
using faest_em_192_s = parameter_set<secpar::s192, 16, owf::aes_em, prg::aes_ctr>;
using faest_em_192_f = parameter_set<secpar::s192, 24, owf::aes_em, prg::aes_ctr>;
using faest_em_256_s = parameter_set<secpar::s256, 22, owf::aes_em, prg::aes_ctr>;
using faest_em_256_f = parameter_set<secpar::s256, 32, owf::aes_em, prg::aes_ctr>;
} // namespace v1

// Macro listing all instances, useful to instantiate tests with all parameter sets
#define ALL_FAEST_V1_INSTANCES                                                                     \
    v1::faest_128_s, v1::faest_128_f, v1::faest_192_s, v1::faest_192_f, v1::faest_256_s,           \
        v1::faest_256_f, v1::faest_em_128_s, v1::faest_em_128_f, v1::faest_em_192_s,               \
        v1::faest_em_192_f, v1::faest_em_256_s, v1::faest_em_256_f

// The FAEST v2 instances (XXX: not finalized yet)
namespace v2
{
    // Also seemed like a decent tradeoff:
    // using faest_128_s = parameter_set<secpar::s128, 10, owf::v2, prg::aes_ctr, prg::aes_ctr, leaf_hash::aes_ctr_stat_bind, 12, {bavc::one_tree, 105}>;

using faest_128_s = parameter_set<secpar::s128, 11, owf::v2, prg::aes_ctr, prg::aes_ctr,
                                  leaf_hash::aes_ctr_stat_bind, 7, {bavc::one_tree, 102}>;
using faest_128_f = parameter_set<secpar::s128, 16, owf::v2, prg::aes_ctr, prg::aes_ctr,
                                  leaf_hash::aes_ctr_stat_bind, 8, {bavc::one_tree, 110}>;
using faest_192_s = parameter_set<secpar::s192, 16, owf::v2, prg::aes_ctr, prg::aes_ctr,
                                  leaf_hash::aes_ctr_stat_bind, 12, {bavc::one_tree, 162}>;
using faest_192_f = parameter_set<secpar::s192, 24, owf::v2, prg::aes_ctr, prg::aes_ctr,
                                  leaf_hash::aes_ctr_stat_bind, 8, {bavc::one_tree, 163}>;
using faest_256_s = parameter_set<secpar::s256, 22, owf::v2, prg::aes_ctr, prg::aes_ctr,
                                  leaf_hash::aes_ctr_stat_bind, 6, {bavc::one_tree, 245}>;
using faest_256_f = parameter_set<secpar::s256, 32, owf::v2, prg::aes_ctr, prg::aes_ctr,
                                  leaf_hash::aes_ctr_stat_bind, 8, {bavc::one_tree, 246}>;

using faest_em_128_s = parameter_set<secpar::s128, 11, owf::v2_em, prg::aes_ctr, prg::aes_ctr,
                                     leaf_hash::aes_ctr, 7, {bavc::one_tree, 103}>;
using faest_em_128_f = parameter_set<secpar::s128, 16, owf::v2_em, prg::aes_ctr, prg::aes_ctr,
                                     leaf_hash::aes_ctr, 8, {bavc::one_tree, 112}>;
using faest_em_192_s = parameter_set<secpar::s192, 16, owf::v2_em, prg::aes_ctr, prg::aes_ctr,
                                     leaf_hash::aes_ctr, 8, {bavc::one_tree, 162}>;
using faest_em_192_f = parameter_set<secpar::s192, 24, owf::v2_em, prg::aes_ctr, prg::aes_ctr,
                                     leaf_hash::aes_ctr, 8, {bavc::one_tree, 176}>;
using faest_em_256_s = parameter_set<secpar::s256, 22, owf::v2_em, prg::aes_ctr, prg::aes_ctr,
                                     leaf_hash::aes_ctr, 6, {bavc::one_tree, 218}>;
using faest_em_256_f = parameter_set<secpar::s256, 32, owf::v2_em, prg::aes_ctr, prg::aes_ctr,
                                     leaf_hash::aes_ctr, 8, {bavc::one_tree, 234}>;
} // namespace v2

// Macro listing all instances, useful to instantiate tests with all parameter sets
#define ALL_FAEST_V2_INSTANCES                                                                     \
    v2::faest_128_s, v2::faest_128_f, v2::faest_192_s, v2::faest_192_f, v2::faest_256_s,           \
        v2::faest_256_f, v2::faest_em_128_s, v2::faest_em_128_f, v2::faest_em_192_s,               \
        v2::faest_em_192_f, v2::faest_em_256_s, v2::faest_em_256_f

#define ALL_FAEST_INSTANCES ALL_FAEST_V1_INSTANCES, ALL_FAEST_V2_INSTANCES

} // namespace faest

#endif
