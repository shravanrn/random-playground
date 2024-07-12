
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <x86intrin.h>

typedef union{
    __m64 m64;
    uint64_t val;
} m64reg;

typedef union{
    __m128i m128;
    uint64_t val[2];
} m128reg;

uint64_t dummy(uint64_t arg) {
    return arg;
}

int main(int argc, char const *argv[])
{

    // uint64_t a[3];
    // a[0] = a[1] = a[2] = 0;

    {
        uint64_t shift = 1;
        uint64_t value = 7;
        uint64_t dest = 0;

        __asm__ __volatile__(
            "shrx %[shift], %[value], %[dest]\n"
            : [dest] "+r" (dest)
            : [shift] "r" (shift), [value] "m" (value)
            :
        );
    }


    {
        m64reg addr;
        addr.val = 0x123456789abcdef0;

        m64reg mask;
        mask.val = 0xffffff0403020100;

        m64reg ret;
        ret.m64 = _mm_shuffle_pi8(addr.m64, mask.m64);
    }

    {
        uint64_t addr = 0x123456789abcdef0;
        uint32_t start = 4;
        uint32_t len = 35;
        uint64_t result = _bextr_u64(addr, start, len);
    }


    {
        m128reg dest;
        m128reg src1;
        src1.val[0] = 0x123456789abcdef0;
        m128reg src2;
        src2.val[0] = 0xe0000000000;
        m128reg mask;
        mask.val[0] = 0xffffff0000000000;
        dest.m128 = _mm_blendv_epi8(src1.m128, src2.m128, mask.m128);
        dummy(dest.val[0]);
        dummy(dest.val[1]);
    }


    {
        uint64_t val = 0xfff000;
        int32_t result = __bsrq(val);
        dummy(result);
    }

    return 0;
}
