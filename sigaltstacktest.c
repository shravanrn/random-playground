#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>

static char* g_alt_stack = 0;

static bool os_has_altstack_installed() {
  /* Use alt stack to handle SIGSEGV from stack overflow */
  /* check for altstack already in place */
  stack_t ss;
  if (sigaltstack(NULL, &ss) != 0) {
    perror("sigaltstack failed");
    abort();
  }

  return !(ss.ss_flags & SS_DISABLE);
}

/* Use alt stack to handle SIGSEGV from stack overflow */
static void* os_allocate_and_install_altstack(void) {
  /* allocate altstack */
  void* alt_stack = malloc(SIGSTKSZ);
  if (alt_stack == NULL) {
    perror("malloc failed");
    abort();
  }

  /* install altstack */
  stack_t ss;
  ss.ss_sp = alt_stack;
  ss.ss_flags = 0;
  ss.ss_size = SIGSTKSZ;
  if (sigaltstack(&ss, NULL) != 0) {
    perror("sigaltstack failed");
    abort();
  }

  return alt_stack;
}

static void os_disable_and_deallocate_altstack(void* alt_stack) {
  /* verify altstack was still in place */
  stack_t ss;
  if (sigaltstack(NULL, &ss) != 0) {
    perror("sigaltstack failed");
    abort();
  }
  if ((ss.ss_flags & SS_DISABLE) || (ss.ss_sp != alt_stack) ||
      (ss.ss_size != SIGSTKSZ)) {
    fprintf(stderr, "Warning: wasm2c altstack was modified unexpectedly\n");
  }

  /* disable & free */
  ss.ss_flags = SS_DISABLE;
  if (sigaltstack(&ss, NULL) != 0) {
    perror("sigaltstack failed");
    abort();
  }
  assert(!os_has_altstack_installed());

  free(alt_stack);
}

int main(int argc, char const *argv[])
{
  printf("Installed: %s\n", os_has_altstack_installed()? "True" : "False");
  printf("Installing\n");
  void* st = os_allocate_and_install_altstack();
  printf("Installed: %s\n", os_has_altstack_installed()? "True" : "False");
  printf("UnInstalling\n");
  os_disable_and_deallocate_altstack(st);
  printf("Installed: %s\n", os_has_altstack_installed()? "True" : "False");
  return 0;
}
