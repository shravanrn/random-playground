#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <immintrin.h> /* compiler intrinsics */

int main(int argc, char const *argv[])
{
    uint64_t length = 0x0000001000000000;
    uint64_t offset = 0x0000011100010110;
    uint64_t p_mask = 0x0fff000fffffffff;
    uint64_t shifted = _pdep_u64(offset, p_mask);
    uint64_t ored = offset | shifted;

    printf("length  : 0x%016" PRIx64 "\n", length);
    printf("offset  : 0x%016" PRIx64 "\n", offset);
    printf("p_mask  : 0x%016" PRIx64 "\n", p_mask);
    printf("shifted : 0x%016" PRIx64 "\n", shifted);
    printf("ored    : 0x%016" PRIx64 "\n", ored);
    return 0;
}
