#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <immintrin.h> /* compiler intrinsics */


#include <sys/auxv.h>
#ifdef __GLIBC__
#include <gnu/libc-version.h>
#endif

#ifdef __linux__

// https://www.man7.org/linux/man-pages/man2/arch_prctl.2.html
#include <asm/prctl.h>    // For ARCH_SET_GS
#include <sys/syscall.h>  // For SYS_arch_prctl
#include <unistd.h>       // For syscall
#define SYSCALL_GET_GS(base_ptr) syscall(SYS_arch_prctl, ARCH_GET_GS, base_ptr)
#define SYSCALL_SET_GS(base) syscall(SYS_arch_prctl, ARCH_SET_GS, base)

#elif defined(__FreeBSD__) || defined(__DragonFly__)

// https://github.com/freebsd/freebsd-src/blob/d6fb9f8ca344bfe47fc79d3ae81112a8bc036307/sys/x86/include/sysarch.h#L142
// https://github.com/DragonFlyBSD/DragonFlyBSD/blob/8506772f4f44fcae9c78e61800e54be0399905f8/sys/cpu/x86_64/include/sysarch.h#L52
#include <machine/sysarch.h>
#define SYSCALL_GET_GS(base_ptr) amd64_get_gsbase(base_ptr)
#define SYSCALL_SET_GS(base) amd64_set_gsbase(base)

#elif defined(__NetBSD__)

// https://github.com/NetBSD/src/blob/ce716eeb9a02c7ecc82ab81d906a970d97432925/sys/arch/x86/include/sysarch.h#L82
#include <machine/sysarch.h>
#define SYSCALL_GET_GS(base_ptr) sysarch(X86_64_SET_GSBASE, base_ptr)
#define SYSCALL_SET_GS(base) sysarch(X86_64_SET_GSBASE, base)

#elif defined(__OpenBSD__)

// No system call available
// https://github.com/openbsd/src/blob/493aa139460d27930d60a92380691ef4e2dde0bd/sys/arch/amd64/include/sysarch.h#L1
#error "Segue not supported on this OS"

#else
#error "Unknown OS"
#endif

#ifndef HWCAP2_FSGSBASE
#define HWCAP2_FSGSBASE        (1 << 1)
#endif

#define size 4 * 1024

int main(int argc, char const *argv[]) {
    char buffer1[size];
    char buffer2[size];

    buffer1[0] = 'g';
    buffer2[0] = 'h';

#if defined(__GLIBC__) && __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 18
  // Check for support for userspace wrgsbase instructions
  unsigned long val = getauxval(AT_HWCAP2);
  wasm_rt_fsgsbase_inst_supported = val & (1 << 1);
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
  unsigned long val = 0;
  if (elf_aux_info(AT_HWCAP2, &val, sizeof(unsigned long)) == 0) {
    wasm_rt_fsgsbase_inst_supported = val & (1 << 1);
  }
#endif

    if (val & HWCAP2_FSGSBASE) {
      printf("FSGSBASE enabled\n");
    } else {
      printf("FSGSBASE disabled\n");
    }

    _writegsbase_u64((uintptr_t)&buffer1[1]);
    char __seg_gs * buffer1_seg = (char __seg_gs *) -1;
    printf("Buffer 1 char expected: %c, got: %c\n", buffer1[0], *buffer1_seg);

  if (SYSCALL_SET_GS((uintptr_t)&buffer2[1]) != 0){
    printf("Syscall SYS_arch_prctl error: %s\n", strerror(errno));
  }

  char __seg_gs * buffer2_seg = (char __seg_gs *) -1;
  printf("Buffer 2 char expected: %c, got: %c\n", buffer2[0], *buffer2_seg);


}
