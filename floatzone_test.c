#define _GNU_SOURCE

#include <assert.h>
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fenv.h>

static char *fe_code_name[] = {
  "FPE_NOOP",
  "FPE_FLTDIV", "FPE_FLTINV", "FPE_FLTOVF", "FPE_FLTUND",
  "FPE_FLTRES", "FPE_FLTSUB", "FPE_INTDIV", "FPE_INTOVF"
  "FPE_UNKNOWN"
};

sigjmp_buf fpe_buf;

void sigfpe_handler (int sig, siginfo_t *sip, ucontext_t *scp)
{
    int fe_code = sip->si_code;

    if (sig == SIGFPE) {
        // printf ("signal:  SIGFPE with code %s\n", fe_code_name[fe_code]);
        longjmp(fpe_buf, 1);
    }

    printf ("Signal is not SIGFPE, it's %i.\n", sig);
    abort();
}

#if defined(__clang__) && \
    (defined(mips) || defined(__mips__) || defined(__mips))
#define FORCE_READ_FLOAT(var) __asm__("" ::"f"(var));
#else
#define FORCE_READ_FLOAT(var) __asm__("" ::"r"(var));
#endif

bool test_add(float a, float b) {
    int error_handler_called = sigsetjmp(fpe_buf, 1);

    if (!error_handler_called) {
        float ret = a + b;
        FORCE_READ_FLOAT(ret);
        return true;
    } else {
        uint32_t a_copy, b_copy;
        memcpy(&a_copy, &a, sizeof(a_copy));
        memcpy(&b_copy, &b, sizeof(b_copy));
        printf("  Error handler called for adding: %p + %p\n", (void*)(uintptr_t)a_copy, (void*)(uintptr_t)b_copy);
        assert(feenableexcept(FE_UNDERFLOW) != -1);
        return false;
    }
}

int main (int argc, char **argv)
{
    double s;
    struct sigaction act;

    act.sa_sigaction = (void(*))sigfpe_handler;
    sigemptyset (&act.sa_mask);
    act.sa_flags = SA_SIGINFO;

    assert(feenableexcept(FE_UNDERFLOW) != -1);

    if (sigaction(SIGFPE, &act, (struct sigaction *)0) != 0)
    {
        perror("Sigaction failed");
        exit(-1);
    }

    uint32_t x_int = 0x0b8b8b8a;
    float x;
    memcpy(&x, &x_int, sizeof(float));

    uint32_t crash_value_int = 0x8b8b8b8b;
    float crash_value;
    memcpy(&crash_value, &crash_value_int, sizeof(float));

    uint32_t crash_value_int2 = 0x8b8b8b89;
    float crash_value2;
    memcpy(&crash_value2, &crash_value_int2, sizeof(float));

    float safe_value = 1.0;
    float zero = 0;

    puts("Computing safe addition");
    test_add(x, safe_value);

    puts("Computing div by zero");
    float ret2 = x / zero;
    FORCE_READ_FLOAT(ret2);

    puts("Computing crash addition");
    test_add(x, crash_value);

    puts("Computing crash addition 2");
    test_add(x, crash_value2);

    puts("Running the full test");
    for (uint64_t y_int = 0; y_int <= UINT32_MAX; y_int++) {
        float y;
        memcpy(&y, &y_int, sizeof(float));
        test_add(x, y);
    }

    puts("Done");

    return 0;
}
