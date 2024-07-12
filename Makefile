CC:=clang
CXX:=clang++
CFLAGS+=-O0 -g
CXXFLAGS+=$(CFLAGS) -std=c++20
LDFLAGS+=-lm

BINARY_FILES=$(filter-out Makefile, $(patsubst ./%, %, $(shell find . -maxdepth 1 -type f ! -name "*.*")))
PRE_FILES=$(patsubst ./%, %, $(shell find . -maxdepth 1 -type f -name "*.pre.cpp"))
OUT_FILES=./dummy $(BINARY_FILES) $(PRE_FILES)

%_32: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -m32 $< -o $@

%.pre.cpp: %.cpp
	$(CXX) -E $< -o $@

segtest: CFLAGS+=-mfsgsbase

pdep_test: CFLAGS+=-mbmi2

bitinst_tests: CFLAGS+=-mssse3 -mbmi -msse4.1

optional_test: optional_test/test.cpp

wrapper_test_single: | wrapper_test_single.pre.cpp

segments_clobber_test: CFLAGS+=-mfsgsbase

clean:
	rm -rf $(OUT_FILES)