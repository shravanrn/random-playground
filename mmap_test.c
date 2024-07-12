#include <stdint.h>
#include <sys/mman.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

static void* os_mmap_at(void* loc, size_t size) {
  int map_prot = PROT_NONE;
  int map_flags = MAP_ANONYMOUS | MAP_PRIVATE;
  if (loc != NULL) {
    map_flags |= MAP_FIXED;
  }
  uint8_t* addr = mmap(loc, size, map_prot, map_flags, -1, 0);
  if (addr == MAP_FAILED)
    return NULL;
  if (loc != NULL && addr != NULL && addr != loc){
    munmap(addr, size);
    return NULL;
  }
  return addr;
}


int main(){
    if (!os_mmap_at((void*)(uintptr_t)0xff000, 4096)){
        perror("os_mmap shadow bitscan failed.");
        abort();
  }
}