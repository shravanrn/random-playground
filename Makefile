hello_world.out: hello_world.c
	gcc -O0 -g $< -o $@

segtest.out: segtest.c
	gcc -mfsgsbase -O0 -g $< -o $@

segtest_32.out: segtest.c
	gcc -m32 -O0 -g $< -o $@

floatzone_test.out: floatzone_test.c
	gcc -O0 -g $< -o $@ -lm

watchpoint_test.out: watchpoint_test.c
	gcc -O0 -g $< -o $@

fp_exceptions_enable.out: fp_exceptions_enable.c
	gcc -O0 -g $< -o $@ -lm

pdep_test.out: pdep_test.c
	gcc -mbmi2 -O0 -g $< -o $@

sigaltstacktest.out: sigaltstacktest.c

.PHONY: optional_test.out
optional_test.out:
	g++ -O0 -g optional_test/test.cpp -o $@

wrapper_test.out: wrapper_test.cpp
	g++ -O0 -g wrapper_test.cpp -o $@

wrapper_test_single.out: wrapper_test_single.cpp
	g++ -O0 -g -E wrapper_test_single.cpp -o wrapper_test_single.pre.cpp
	g++ -O0 -g wrapper_test_single.cpp -o $@

wrapper_test_cpp20.out: wrapper_test_cpp20.cpp
	g++ -O0 -g -std=c++20 wrapper_test_cpp20.cpp -o $@

segments_clobber_test.out: segments_clobber_test.c
	clang $< -o $@ -O0 -mfsgsbase -g

clean:
	rm -rf ./*.out