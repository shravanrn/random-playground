#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <immintrin.h> /* compiler intrinsics */

#include <asm/ldt.h>         /* Definition of struct user_desc */
#include <sys/mman.h>
#include <sys/syscall.h>     /* Definition of SYS_* constants */
#include <unistd.h>

#include <asm/prctl.h>        /* Definition of ARCH_* constants */

#include <sys/auxv.h>
#include <elf.h>

#ifndef HWCAP2_FSGSBASE
#define HWCAP2_FSGSBASE        (1 << 1)
#endif

int modify_ldt(int func, void *ptr, unsigned long bytecount) {
  return syscall(__NR_modify_ldt, func, ptr, bytecount);
}

static void* os_mmap_aligned(void* addr,
                             size_t requested_length,
                             int prot,
                             int flags,
                             size_t alignment,
                             size_t alignment_offset)
{
  size_t padded_length = requested_length + alignment + alignment_offset;
  uintptr_t unaligned = (uintptr_t)mmap(addr, padded_length, prot, flags, -1, 0);

  if (!unaligned || unaligned == -1) {
    return (void*)0;
  }

  // Round up the next address that has addr % alignment = 0
  const size_t alignment_corrected = alignment == 0 ? 1 : alignment;
  uintptr_t aligned_nonoffset =
    (unaligned + (alignment_corrected - 1)) & ~(alignment_corrected - 1);

  // Currently offset 0 is aligned according to alignment
  // Alignment needs to be enforced at the given offset
  uintptr_t aligned = 0;
  if ((aligned_nonoffset - alignment_offset) >= unaligned) {
    aligned = aligned_nonoffset - alignment_offset;
  } else {
    aligned = aligned_nonoffset - alignment_offset + alignment;
  }

  // Sanity check
  if (aligned < unaligned ||
      (aligned + (requested_length - 1)) > (unaligned + (padded_length - 1)) ||
      (aligned + alignment_offset) % alignment_corrected != 0) {
    munmap((void*)unaligned, padded_length);
    return NULL;
  }

  {
    size_t unused_front = aligned - unaligned;
    if (unused_front != 0) {
      munmap((void*)unaligned, unused_front);
    }
  }

  {
    size_t unused_back =
      (unaligned + (padded_length - 1)) - (aligned + (requested_length - 1));
    if (unused_back != 0) {
      munmap((void*)(aligned + requested_length), unused_back);
    }
  }

  return (void*)aligned;
}

static uint16_t set_ldt(void* address, size_t size) {
    struct user_desc ud;
    ud.entry_number = LDT_ENTRIES - 1;
    ud.contents = MODIFY_LDT_CONTENTS_DATA;
    ud.read_exec_only = 0;
    ud.seg_32bit = 0;
    ud.seg_not_present = 0;
    ud.useable = 1;
    ud.base_addr = (uintptr_t) address;
    ud.limit = size - 1;
    ud.limit_in_pages = 0;

    if (modify_ldt(1, &ud, sizeof ud) == -1) {
        printf("Error on modify_ldt\n");
        abort();
    }

    return (ud.entry_number << 3) | 0x7;
}

void test()
{
    const uint64_t mb = 1024 * 1024;
    const uint64_t gb = 1024 * 1024 * 1024;
    const uint64_t size = 4 * mb;

    char* buffer1 = os_mmap_aligned(0, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, size, 0);
    char* buffer2 = os_mmap_aligned(0, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, size, 0);

    buffer1[0] = 'g';
    buffer2[0] = 'h';
    printf("Buffers: %p, %p\n", buffer1, buffer2);

#if __x86_64__
    unsigned val = getauxval(AT_HWCAP2);

    if (val & HWCAP2_FSGSBASE) {
      printf("FSGSBASE enabled\n");
    }

    _writegsbase_u64((uintptr_t)&buffer1[1]);
    char __seg_gs * buffer1_seg = (char __seg_gs *) -1;
    printf("Buffer 1 char expected: %c, got: %c\n", buffer1[0], *buffer1_seg);
#else
    uint16_t sel = set_ldt(buffer1, size);
    asm("\n");
#endif

  if (syscall(SYS_arch_prctl, ARCH_SET_GS, (uintptr_t)&buffer2[1]) != 0){
    printf("Syscall SYS_arch_prctl error: %s\n", strerror(errno));
  }

  char __seg_gs * buffer2_seg = (char __seg_gs *) -1;
  printf("Buffer 2 char expected: %c, got: %c\n", buffer2[0], *buffer2_seg);


}

int main(int argc, char const *argv[]) {
    test();
    return 0;
}