COMMON_LD_FLAGS ?= -O2 -march=native -mtune=native -DNDEBUG # Benchmark
#COMMON_LD_FLAGS ?= -O2 -march=native -mtune=native -ggdb -fsanitize=address # Debug Fast
#COMMON_LD_FLAGS ?= -O0 -march=native -mtune=native -ggdb -fsanitize=address -fsanitize=undefined # Debug Slow
COMMON_CC_FLAGS ?= -pedantic-errors -Wall -Wextra -Wno-ignored-attributes $(COMMON_LD_FLAGS)

CPPFLAGS += -MMD -MP -MF $*.d
CFLAGS ?= -std=c11 $(COMMON_CC_FLAGS)
CXXFLAGS ?= -std=c++20 $(COMMON_CC_FLAGS)
LDFLAGS += -lcrypto $(COMMON_LD_FLAGS)

CP_L = cp -l
MKDIR_P = mkdir -p

keccak_sources = \
	$(wildcard XKCP/lib/high/Keccak/FIPS202/KeccakHash.*)\
	$(wildcard XKCP/lib/high/Keccak/KeccakSponge.*)\
	$(wildcard XKCP/lib/common/*)\
	$(wildcard XKCP/lib/low/common/*)\
	$(wildcard XKCP/lib/low/KeccakP-1600/common/*)
keccak_avx2_sources = \
	$(wildcard XKCP/lib/low/KeccakP-1600/AVX2/*)\
	$(wildcard XKCP/lib/low/KeccakP-1600-times2/SIMD128/KeccakP-1600-times2-*)\
	$(wildcard XKCP/lib/low/KeccakP-1600-times2/SIMD128/SSSE3-u2/SIMD128-config.h)\
	$(wildcard XKCP/lib/low/KeccakP-1600-times4/AVX2/KeccakP-1600-times4-*)\
	$(wildcard XKCP/lib/low/KeccakP-1600-times4/AVX2/u12/SIMD256-config.h)\
	$(wildcard XKCP/lib/low/KeccakP-1600-times8/fallback-on4/*)
common_headers = Catch2/extras/catch_amalgamated.hpp
common_sources = $(common_headers) Catch2/extras/catch_amalgamated.cpp

shared_sources = $(filter-out randomness_randombytes.c randomness_os.c,$(wildcard *.c *.h *.in *.inc)) $(keccak_sources)
#opt_sources = $(wildcard opt/*.c opt/*.h)
ref_sources = $(shared_sources) $(wildcard ref/*.c ref/*.h)
avx2_sources = $(shared_sources) $(wildcard avx2/*.c avx2/*.h) $(keccak_avx2_sources)
avx2_vaes_sources = $(shared_sources) $(wildcard avx2_vaes/*.c avx2_vaes/*.h) $(keccak_avx2_sources)
test_sources = $(wildcard test/*.cpp test/*.hpp) $(common_headers) randomness_os.c
api_test_sources = test/api_test.c randomness_os.c
kat_sources = test/rng.c test/rng.h test/PQCgenKAT_sign.c randomness_randombytes.c

submission_versions = sec128_cccs_11 sec128_cccs_16 sec192_cccs_16 sec192_cccs_24 sec256_cccs_22 sec256_cccs_32 sec128_eccs_11 sec128_eccs_16 sec192_eccs_16 sec192_eccs_24 sec256_eccs_22 sec256_eccs_32

all:
.PHONY: all

security_params = 128 192 256
#taus_128 = 10 11 12 13 14 15 16 17 18 19 20 22 24 26 28 30 32
#taus_192 = 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 32 34 36 38 40 42 44 46 48
#taus_256 = 20 22 24 26 28 30 32 34 36 38 40 44 48 52 56 60 64
taus_128 = 10 11 16 32
taus_192 = 15 16 24 48
taus_256 = 20 22 32 52 64

# Format: long name (for macro definition), single letter name.
ciphers = AES_CTR,c RIJNDAEL_EVEN_MANSOUR,e
tree_prgs = $(ciphers)
leaf_prgs = $(ciphers) SHAKE,s

comma=,
define first
$(firstword $(subst $(comma), ,$(1)))
endef
define second
$(word 2,$(subst $(comma), ,$(1)))
endef

# Format: name,security_param,owf,owf_letter,prg,tree_prg,leaf_prg,tau
settings = \
	$(foreach security_param,$(security_params),\
		$(foreach owf,$(ciphers),\
			$(foreach prg,$(ciphers),\
				$(foreach tree_prg,$(tree_prgs),\
					$(foreach leaf_prg,$(leaf_prgs),\
						$(foreach tau,$(taus_$(security_param)),\
							$(if $(and $(findstring 192,$(security_param)),$(findstring RIJNDAEL,$(prg)$(tree_prg)$(leaf_prg))),,\
								sec$(security_param)_$(call second,$(owf))$(call second,$(prg))$(call second,$(tree_prg))$(call second,$(leaf_prg))_$(tau),$(security_param),$(call first,$(owf)),$(call second,$(owf)),$(call first,$(prg)),$(call first,$(tree_prg)),$(call first,$(leaf_prg)),$(tau)\
							)\
						)\
					)\
				)\
			)\
		)\
	)

get_faest_name = $(subst sec128_cccs_11,faest_128s,$(subst sec128_cccs_16,faest_128f,$(subst sec192_cccs_16,faest_192s,$(subst sec192_cccs_24,faest_192f,$(subst sec256_cccs_22,faest_256s,$(subst sec256_cccs_32,faest_256f,$(subst sec128_eccs_11,faest_em_128s,$(subst sec128_eccs_16,faest_em_128f,$(subst sec192_eccs_16,faest_em_192s,$(subst sec192_eccs_24,faest_em_192f,$(subst sec256_eccs_22,faest_em_256s,$(subst sec256_eccs_32,faest_em_256f,$(1)))))))))))))

define link-recipe
$(1)/$(notdir $(2)) : $(2) | $(dir $(1)/$(notdir $(2)))
	rm -f $$@
	$(CP_L) $$< $$@
endef

define config-recipe
$(1)/% : %.in | $(1)/
	$(let fullname security_param owf owf_letter prg tree_prg leaf_prg tau,$(subst $(comma), ,$(2)),\
	$(let name,$(call get_faest_name,$(name)),\
	$(let iv_bits,$(if $(findstring RIJNDAEL,$(owf)),$(security_param),$(if $(findstring 128,$(security_param)),128,256)),\
	$(let sk_bytes,$(shell expr "(" $(security_param) "+" $(iv_bits) ")" "/" 8),\
	$(let pk_bytes,$(if $(and $(findstring AES_CTR,$(owf)),$(intcmp $(security_param),192)),64,$(sk_bytes)),\
	sed $(foreach substitution,\
		"%VERSION%/$(name)"\
		"%SECURITY_PARAM%/$(security_param)"\
		"%OWF%/$(owf)"\
		"%PRG%/$(prg)"\
		"%TREE_PRG%/$(tree_prg)"\
		"%LEAF_PRG%/$(leaf_prg)"
		"%TAU%/$(tau)"
		"%SECRETKEYBYTES%/$(sk_bytes)"\
		"%PUBLICKEYBYTES%/$(pk_bytes)"\
		,\
		-e "s/"$(substitution)"/g" \
	) -e "s/%SIGBYTES%/`python3 scripts/get_signature_size.py $(security_param) $(tau) $(owf_letter)`/g" $$< > $$@ \
	)))))
endef



# canned recipe for a variant
# - arguments:
# 	$(1): architecture (e.g. "ref", "avx2")
# 	$(2): setting name (e.g. "sec128_cccc_10_avx2")
# 	$(3): path (e.g. "Additional_Implementations/sec128_cccc_10_avx2")
# 	$(4): setting (comma-separated list of the form name,security_param,owf,owf_letter,prg,tree_prg,leaf_prg,tau)
define full-recipe

# $(1)_sources contains the shared and archtecture-specific source files

$(2)_objects = $$(foreach source,$$(patsubst %.c,%.o,$$(filter %.c,$$($(1)_sources))),$(3)/$$(notdir $$(source)))
$(2)_asm_objects = $$(foreach obj,$$(patsubst %.s,%.o,$$(filter %.s,$$($(1)_sources))),$(3)/$$(notdir $$(obj)))
$(2)_headers = $$(foreach header,$$(filter %.h %.inc %.macros,$$(patsubst %.in,%,$$($(1)_sources))),$(3)/$$(notdir $$(header)))
$(2)_test_headers = $$(foreach header,$$(filter %.hpp,$$(test_sources)),$(3)/$$(notdir $$(header)))
$(2)_test_objects = $$(foreach obj,$$(patsubst %.c,%.o,$$(patsubst %.cpp,%.o,$$(filter %.cpp %.c,$$(test_sources)))) $$(common_objects),$(3)/$$(notdir $$(obj)))
$(2)_kat_headers = $$(foreach header,$$(filter %.h,$$(kat_sources)),$(3)/$$(notdir $$(header)))
$(2)_kat_objects = $$(foreach obj,$$(patsubst %.c,%.o,$$(filter %.c,$$(kat_sources))),$(3)/$$(notdir $$(obj)))
$(2)_api_test_headers = $$(foreach header,$$(filter %.h,$$(api_test_sources)),$(3)/$$(notdir $$(header)))
$(2)_api_test_objects = $$(foreach obj,$$(patsubst %.c,%.o,$$(filter %.c,$$(api_test_sources))),$(3)/$$(notdir $$(obj)))
$(2)_targets = $$($(2)_objects) $$($(2)_asm_objects) $$($(2)_headers) $(3)/$(2)_test $(3)/PQCgenKAT_sign $(3)/api_test $(3)/Makefile
$(2)_depfiles = $$(patsubst %.o,%.d,$$($(2)_objects) $$($(2)_test_objects) $$($(2)_kat_objects) api_test.o)

# hard link all source files into the variant directory. Also copy common object files.
$$(foreach src,$$($(1)_sources) $$(test_sources) $$(common_objects) $$(kat_sources) test/api_test.c,$$(eval $$(call link-recipe,$(3),$$(src))))

# generate config.h with the setting-specific constants
$(eval $(call config-recipe,$(3),$(4)))

# object files depend on the headers (build order only)
headers-$(2) : $$($(2)_headers)
.PHONY: headers-$(2)
$$($(2)_objects)) : | headers-$(2)

# same for test files
test-headers-$(2) : $$($(2)_test_headers) $$($(2)_kat_headers) $$($(2)_api_test_headers)
.PHONY: test-headers-$(2)
$$($(2)_test_objects)) $$($(2)_kat_objects)) $$($(2)_api_test_objects)) $(3)/api_test.o : | headers-$(2) test-headers-$(2)

# target for test binary
$(3)/$(2)_test : $$($(2)_test_objects) $$($(2)_objects) $$($(2)_asm_objects)
	$(CXX) -o $$@ $(LDFLAGS) $$^ $(LOADLIBES) $(LDLIBS)

$(3)/PQCgenKAT_sign : $$($(2)_kat_objects) $$($(2)_objects) $$($(2)_asm_objects)
	$(CC) -o $$@ $(LDFLAGS) $$^ $(LOADLIBES) $(LDLIBS)

$(3)/api_test : $$($(2)_api_test_objects) $$($(2)_objects) $$($(2)_asm_objects)
	$(CC) -o $$@ $(LDFLAGS) $$^ $(LOADLIBES) $(LDLIBS)

# targets to create (sub)directories
$(3)/:
	$$(MKDIR_P) $$@
$(3)/%/:
	$$(MKDIR_P) $$@

# target for the variant directory
$(2) : $$($(2)_targets) $(3)/Makefile
.PHONY : $(2)
all : $(2)

# magic to generate dependency files
# https://make.mad-scientist.net/papers/advanced-auto-dependency-generation/
$$($(2)_depfiles):
include $$(wildcard $$($(2)_depfiles))
endef

# Compile common object files in a subfolder
common_objects = $(foreach obj,$(patsubst %.cpp,%.o,$(filter %.cpp,$(common_sources))),Common/$(notdir $(obj)))
$(foreach src,$(common_sources),$(eval $(call link-recipe,Common,$(src))))

headers-common : $(foreach header,$(common_headers),Common/$(notdir $(header)))
.PHONY: headers-common
$(common_objects) : | headers-common

Common/:
	$(MKDIR_P) $@

#$(foreach setting,$(settings),\
#	$(let name,$(call first,$(setting)),\
#		$(eval $(call full-recipe,ref,$(name)_ref,Reference_Implementation/$(name),$(setting)))\
#	)\
#)
$(foreach setting,$(settings),\
	$(let name,$(call first,$(setting)),\
		$(eval $(call full-recipe,avx2,$(name)_avx2,Additional_Implementations/$(name)_avx2,$(setting)))\
	)\
)
#$(foreach setting,$(settings),\
#	$(let name,$(call first,$(setting)),\
#		$(eval $(call full-recipe,avx2_vaes,$(name)_avx2_vaes,Additional_Implementations/$(name)_avx2_vaes,$(setting)))\
#	)\
#)
# TODO: AVX2 + VAES
# TODO: AVX-512

dist: $(foreach version,$(submission_versions),$(version)_avx2)
	rm -rf Submission
	mkdir Submission
	mkdir $(foreach version,$(submission_versions),Submission/$(call get_faest_name,$(version)))
	$(foreach version,$(submission_versions),$(CP_L) Additional_Implementations/$(version)_avx2/*.{c,h,inc,macros,s} Additional_Implementations/$(version)_avx2/Makefile Submission/$(call get_faest_name,$(version))/ &&) true

.PHONY: dist

clean:
	rm -rf Reference_Implementation Additional_Implementations Common
.PHONY: clean
