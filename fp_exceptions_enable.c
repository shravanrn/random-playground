#define _GNU_SOURCE

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <fenv.h>

// x87 fpu
#define getx87cr(x)    __asm ("fnstcw %0" : "=m" (x));
#define setx87cr(x)    __asm ("fldcw %0"  : "=m" (x));
#define getx87sr(x)    __asm ("fnstsw %0" : "=m" (x));

// SIMD, gcc with Intel Core 2 Duo uses SSE2(4)
#define getmxcsr(x)    __asm ("stmxcsr %0" : "=m" (x));
#define setmxcsr(x)    __asm ("ldmxcsr %0" : "=m" (x));

static char *fe_code_name[] = {
  "FPE_NOOP",
  "FPE_FLTDIV", "FPE_FLTINV", "FPE_FLTOVF", "FPE_FLTUND",
  "FPE_FLTRES", "FPE_FLTSUB", "FPE_INTDIV", "FPE_INTOVF"
  "FPE_UNKNOWN"
};


void
fhdl (int sig, siginfo_t *sip, ucontext_t *scp)
{
  int fe_code = sip->si_code;
  unsigned int excepts = fetestexcept (FE_ALL_EXCEPT);

  /* ... see complete code in link above ... */

    if ( sig == SIGFPE )
    {
        unsigned short x87cr,x87sr;
        unsigned int mxcsr;

        getx87cr (x87cr);
        getx87sr (x87sr);
        getmxcsr (mxcsr);
        printf ("X87CR:   0x%04X\n", x87cr);
        printf ("X87SR:   0x%04X\n", x87sr);
        printf ("MXCSR:   0x%08X\n", mxcsr);
        // ....

        printf ("signal:  SIGFPE with code %s\n", fe_code_name[fe_code]);
        printf ("invalid flag:    0x%04X\n", excepts & FE_INVALID);
        printf ("divByZero flag:  0x%04X\n", excepts & FE_DIVBYZERO);
  } else {
    printf ("Signal is not SIGFPE, it's %i.\n", sig);
  }

  abort();
}

int main (int argc, char **argv)
{
    double s;
    struct sigaction act;

    act.sa_sigaction = (void(*))fhdl;
    sigemptyset (&act.sa_mask);
    act.sa_flags = SA_SIGINFO;


//  printf ("Old divByZero exception: 0x%08X\n", feenableexcept (FE_DIVBYZERO));
    printf ("Old invalid exception:   0x%08X\n", feenableexcept (FE_INVALID));
    printf ("New fp exception:        0x%08X\n", fegetexcept ());

    // set handler
    if (sigaction(SIGFPE, &act, (struct sigaction *)0) != 0)
    {
        perror("Yikes");
        exit(-1);
    }

//  s = 1.0 / 0.0;  // FE_DIVBYZERO
    s = 0.0 / 0.0;  // FE_INVALID
    return 0;
}
