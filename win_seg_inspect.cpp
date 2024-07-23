// From http://ghettohaxxx-blog.azurewebsites.net/executing-bsd-elfs-in-windows/

#define _CRT_SECURE_NO_WARNINGS

#include <thread>
#include <cstdint>
#include <cstdio>
#include <vector>
#include <Windows.h>

#include <immintrin.h> /* compiler intrinsics */


static void show_regs(const char *x) {
    uint64_t fsbase = _readfsbase_u64();
    uint64_t gsbase = _readgsbase_u64();
    puts(x);
    printf("  fsbase %16llx gsbase %16llx\n",
        fsbase, gsbase);
}

int main() {
    show_regs("main thread:");
    DWORD grab_size = 0x100000;
    auto p = (uint8_t *)VirtualAlloc(
        (PVOID)_readfsbase_u64(), grab_size,
        MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    printf("grabbed %p : %p\n", p, p + grab_size);
    for (int i = 0; i < 10; i++) {
        std::thread x([&]() {
            char b[10];
            sprintf(b, "thread %i:", i);
            show_regs(b);
        });
        // keep execution sequential
        if (x.joinable()) {
            x.join();
        }
    }
    return 0;
}