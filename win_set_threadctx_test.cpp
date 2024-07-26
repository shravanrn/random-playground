#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <thread>

#include <immintrin.h> /* compiler intrinsics */

#ifdef _WIN32
#include <windows.h>
#include <winternl.h>
#else
#include <unistd.h> // usleep
#include <sched.h>  // sched_yield
#include <sys/mman.h> // mmap
#endif

#define Check(cond) if (!(cond)) { puts("Condition check " #cond "failed\n"); abort(); }

void my_sleep(uint32_t ms) {
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

 void debugger_thread(DWORD tid) {

     HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_ALL_ACCESS, TRUE, tid);

     if (hThread == 0) {
         printf("OpenThread error=%u\n", GetLastError());
         abort();
     }

     if (SuspendThread(hThread) == (DWORD)-1) {
         printf("SuspendThread error=%u\n", GetLastError());
         abort();
     }

     CONTEXT context{ 0 };
     context.ContextFlags = CONTEXT_ALL;
     if (GetThreadContext(hThread, &context) == 0) {
         printf("GetThreadContext error=%u\n", GetLastError());
         abort();
     }

     printf("FS seg: %d,GS seg: %d\n", (int)context.SegFs, (int)context.SegGs);

     if (ResumeThread(hThread) == (DWORD)-1) {
         printf("ResumeThread error=%u\n", GetLastError());
         abort();
     }
 }


int main(int argc, char const *argv[])
{
    printf("Sizeof(PEB) = %llu, Sizeof(TEB) = %llu\n", (long long unsigned) sizeof(PEB), (long long unsigned) sizeof(TEB));
    // Init check
    uintptr_t fs_init = get_fs_base();
    uintptr_t gs_init = get_gs_base();
    printf("Init seg value, FS: %p, GS: %p\n", (void*) fs_init, (void*) gs_init);

    int seg_num = 2;
    // puts("Which segment do you want to clobber: fs (1), gs (2)?\n");
    // scanf("%d",&seg_num);
    int clobber_fs = seg_num == 1;
    int clobber_gs = seg_num == 2;

    Check(clobber_fs || clobber_gs);

    // clobber check
    const size_t size = 4096 * 2;
    uint64_t* test_buff = (uint64_t*) my_mmap(0, size, true, true);
#ifdef _WIN32
    if (clobber_gs) {
        printf("Copying contents of existing gs segment from %p to %p\n", (void*) gs_init, test_buff);
        memcpy(test_buff, (void*) gs_init, size);
        for (size_t i = 0; i < (size / 8); i++) {
            if (test_buff[i] == gs_init) {
                test_buff[i] = (uintptr_t) test_buff;
                printf("Replacing gs segment address at index %llu\n", (long long unsigned) i);
            }
        }
    }
#endif
    //my_mprotect(test_buff, size, true, false);

    DWORD main_tid = GetCurrentThreadId();

    std::thread x([&]() {
        debugger_thread(main_tid);
    });

    if (x.joinable()) {
        x.join();
    }

    uintptr_t clobber_test_val = (uintptr_t)test_buff;
    if (clobber_fs){ set_fs_base(clobber_test_val); }
    if (clobber_gs){ set_gs_base(clobber_test_val); }
    printf("After clobber , FS: %p, GS: %p\n", (void*) get_fs_base(), (void*) get_gs_base());


    my_sleep(100);
    void* fs_changed_sleep =  (void*) get_fs_base();
    void* gs_changed_sleep = (void*) get_gs_base();
    if (clobber_fs){ set_fs_base(clobber_test_val); }
    if (clobber_gs){ set_gs_base(clobber_test_val); }

    my_yield();
    void* fs_changed_yield =  (void*) get_fs_base();
    void* gs_changed_yield = (void*) get_gs_base();
    if (clobber_fs){ set_fs_base(clobber_test_val); }
    if (clobber_gs){ set_gs_base(clobber_test_val); }

    FILE *f = fopen("tmp.txt", "w");
    Check(f != 0);
    fputs("Hello\n", f);
    Check(fclose(f) == 0);
    void* fs_changed_fio =  (void*) get_fs_base();
    void* gs_changed_fio = (void*) get_gs_base();
    if (clobber_fs){ set_fs_base(clobber_test_val); }
    if (clobber_gs){ set_gs_base(clobber_test_val); }


    printf("After sleeping, FS: %p, GS: %p\n", fs_changed_sleep, gs_changed_sleep);
    printf("After yielding, FS: %p, GS: %p\n", fs_changed_yield, gs_changed_yield);
    printf("After file I/O, FS: %p, GS: %p\n", fs_changed_fio, gs_changed_fio);

    printf("Done\n");
    return 0;
}
