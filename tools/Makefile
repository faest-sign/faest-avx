CC?=gcc
CXX?=g++
CFLAGS+= -g -O3 -std=c23 -march=native -mtune=native -flto=auto
CXXFLAGS+= -g -O3 -std=c++23 -march=native -mtune=native -flto=auto
CPPFLAGS+= -I. -Isha3 -DHAVE_OPENSSL
LDFLAGS+= -flto=auto

SOURCES=$(filter-out randomness_os.c randomness_randombytes.c,$(wildcard *.c)) $(wildcard *.cpp) $(wildcard avx2/*.c) $(wildcard avx2/*.cpp) $(wildcard sha3/*.c) $(wildcard sha3/*.s)
EXECUTABLE_APITEST=tests/api_test
EXECUTABLE_NISTKAT=NIST-KATs/PQCgenKAT_sign
LIBFAEST=libfaest.a
NISTKAT_SOURCES=$(wildcard NIST-KATs/*.c)

all: $(LIBFAEST) $(EXECUTABLE_APITEST) $(EXECUTABLE_NISTKAT)

$(LIBFAEST): $(SOURCES:.c=.o) $(SOURCES:.cpp=.o) $(SOURCES:.s=.o)
	ar rcs $@ $^

$(EXECUTABLE_NISTKAT): CPPFLAGS+=-DHAVE_RANDOMBYTES
$(EXECUTABLE_NISTKAT): CFLAGS+=-Wno-sign-compare -Wno-unused-but-set-variable -Wno-unused-parameter -Wno-unused-result

$(EXECUTABLE_APITEST): $(EXECUTABLE_APITEST).o $(LIBFAEST) randomness_os.o
	$(CC) $(CPPFLAGS) $(LDFLAGS) $^ -lcrypto -o $@

$(EXECUTABLE_NISTKAT): $(NISTKAT_SOURCES:.c=.o) $(LIBFAEST) randomness_randombytes.o
	$(CC) $(CPPFLAGS) $(LDFLAGS) $^ -lcrypto -o $@

clean:
	rm -f \
		$(wildcard *.o) \
		$(wildcard sha3/*.o) \
		$(wildcard NIST-KATs/*.o) \
		$(wildcard tests/*.o) \
		$(LIBFAEST) $(EXECUTABLE_APITEST) $(EXECUTABLE_NISTKAT)
