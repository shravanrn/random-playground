#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include <immintrin.h> /* compiler intrinsics */

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h> // usleep
#include <sched.h>  // sched_yield
#include <sys/mman.h> // mmap
#endif

#define Check(cond) if (!(cond)) { puts("Condition check " #cond "failed\n"); abort(); }

void my_sleep(uint64_t ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    Check(usleep(ms * 1000) == 0);
#endif
}

void my_yield() {
#ifdef _WIN32
    SwitchToThread();
#else
    Check(sched_yield() == 0);
#endif
}

uintptr_t get_fs_base() {
    return _readfsbase_u64();
}

void set_fs_base(uintptr_t value) {
    _writefsbase_u64(value);
}

uintptr_t get_gs_base() {
    return _readgsbase_u64();
}

void set_gs_base(uintptr_t value) {
    _writegsbase_u64(value);
}

void* my_mmap(void* hint, size_t size, bool readable, bool writeable) {
  if (writeable && ! readable) {
    abort();
  }
#ifdef _WIN32
  DWORD map_prot = PAGE_NOACCESS;
  if (writeable) {
    map_prot = PAGE_READWRITE;
  } else {
    map_prot = PAGE_READONLY;
  }

  void* addr = VirtualAlloc(hint, size, MEM_COMMIT | MEM_RESERVE, map_prot);
  if (addr == NULL) {
    abort();
  }
#else
  int map_prot = PROT_NONE;
  if (readable) {
    map_prot |= PROT_READ;
  }
  if (writeable) {
    map_prot |= PROT_WRITE;
  }

  void* addr = mmap(hint, size, MAP_ANONYMOUS | MAP_PRIVATE, map_flags, -1, 0);
  if (addr == MAP_FAILED) {
    abort();
  }
#endif
  return addr;
}

void my_mprotect(void* addr, size_t size, bool readable, bool writeable) {
  if (writeable && ! readable) {
    abort();
  }
#ifdef _WIN32
  DWORD map_prot = PAGE_NOACCESS;
  if (writeable) {
    map_prot = PAGE_READWRITE;
  } else {
    map_prot = PAGE_READONLY;
  }

  DWORD map_old_prot;
  if (VirtualProtect(addr, size, map_prot, &map_old_prot) == 0) {
      abort();
  }
#else
  int map_prot = PROT_NONE;
  if (readable) {
    map_prot |= PROT_READ;
  }
  if (writeable) {
    map_prot |= PROT_WRITE;
  }
  if (mprotect(addr, size, map_prot) != 0) {
      abort();
  }
#endif
}


int main(int argc, char const *argv[])
{
    // Init check
    uintptr_t fs_init = get_fs_base();
    uintptr_t gs_init = get_gs_base();
    printf("FS: %p, GS: %p\n", (void*) fs_init, (void*) gs_init);

    // yield check
    uintptr_t fs_changed_yield = fs_init;
    uintptr_t gs_changed_yield = gs_init;
    for(int i = 0; i < 100; i++) {
        my_sleep(10);
        my_yield();
        uintptr_t fs_curr = get_fs_base();
        uintptr_t gs_curr = get_gs_base();

        if (fs_curr != fs_init) { fs_changed_yield = fs_curr; }
        if (gs_curr != gs_init) { gs_changed_yield = gs_curr; }
    }

    printf("After yielding, FS: %p, GS: %p\n", (void*) fs_changed_yield, (void*) gs_changed_yield);

    int seg_num;
    puts("Which segment do you want to clobber: fs (1), gs (2)?\n");
    scanf("%d",&seg_num);
    int clobber_fs = seg_num == 1;
    int clobber_gs = seg_num == 2;

    Check(clobber_fs || clobber_gs);


    // clobber check
    const size_t size = 4096 * 1024;
    char* test_buff = my_mmap(0, size, true, true);
#ifdef _WIN32
    if (clobber_gs) {
        printf("Copying contents of existing gs segment from %p to %p\n", (void*) gs_init, test_buff);
        memcpy(test_buff, (void*) gs_init, 4096);
    }
#endif
    my_mprotect(test_buff, size, true, false);

    uintptr_t clobber_test_val = (uintptr_t)test_buff;
    printf("Clobbering segments to %p\n", (void*) clobber_test_val);

    if (clobber_fs){ set_fs_base(clobber_test_val); }
    if (clobber_gs){ set_gs_base(clobber_test_val); }

    printf("After clobbering, FS: %p, GS: %p\n", (void*) get_fs_base(), (void*) get_gs_base());

    uintptr_t fs_changed_clobber = clobber_test_val;
    uintptr_t gs_changed_clobber = clobber_test_val;
    for(int i = 0; i < 100; i++) {
        my_sleep(10);
        my_yield();

        FILE *f = fopen("tmp.txt", "w");
        Check(f != 0);
        fputs("Hello\n", f);
        Check(fclose(f) == 0);

        uintptr_t fs_curr = get_fs_base();
        uintptr_t gs_curr = get_gs_base();

        if (fs_curr != clobber_test_val) { fs_changed_clobber = fs_curr; }
        if (gs_curr != clobber_test_val) { gs_changed_clobber = gs_curr; }
    }

    printf("After clobbering and yielding, FS: %p, GS: %p\n", (void*) fs_changed_clobber, (void*) gs_changed_clobber);

    printf("Done\n");

    return 0;
}
