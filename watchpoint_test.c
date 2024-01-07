// #include <sys/ptrace.h>
// #include <sys/types.h>
// #include <sys/wait.h>
// #include <unistd.h>
// #include <sys/user.h>   /* For constants
//                                    ORIG_EAX etc */

// #include <stdio.h>
// #include <unistd.h> // getpid

// int main()
// {   pid_t child;
//     long orig_eax;
//     child = fork();
//     if(child == 0) {
//         printf("Child PIDs: %lu, %lu\n", (long unsigned) child, (long unsigned) getpid());
//         ptrace(PTRACE_TRACEME, 0, NULL, NULL);
//         execl("/bin/ls", "ls", NULL);
//     }
//     else {
//         wait(NULL);
//         printf("Parent PIDs: %lu, %lu\n", (long unsigned) child, (long unsigned) getpid());
//         orig_eax = ptrace(PTRACE_PEEKUSER,
//                           child, 120,
//                           NULL);
//         printf("The child made a "
//                "system call %ld\n", orig_eax);
//         ptrace(PTRACE_CONT, child, NULL, NULL);
//     }
//     return 0;
// }

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <unistd.h> // getpid
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <signal.h>

typedef struct {
    uint64_t  dr0_local      : 1;
    uint64_t  dr0_global     : 1;
    uint64_t  dr1_local      : 1;
    uint64_t  dr1_global     : 1;
    uint64_t  dr2_local      : 1;
    uint64_t  dr2_global     : 1;
    uint64_t  dr3_local      : 1;
    uint64_t  dr3_global     : 1;
    uint64_t  le             : 1;
    uint64_t  ge             : 1;
    uint64_t  reserved_10    : 1;
    uint64_t  rtm            : 1;
    uint64_t  reserved_12    : 1;
    uint64_t  gd             : 1;
    uint64_t  reserved_14_15 : 2;
    uint64_t  dr0_break      : 2;
    uint64_t  dr0_len        : 2;
    uint64_t  dr1_break      : 2;
    uint64_t  dr1_len        : 2;
    uint64_t  dr2_break      : 2;
    uint64_t  dr2_len        : 2;
    uint64_t  dr3_break      : 2;
    uint64_t  dr3_len        : 2;
} dr7_t;

/* This matches the 64bit FXSAVE format as defined by AMD. It is the same
   as the 32bit format defined by Intel, except that the selector:offset pairs
   for data and eip are replaced with flat 64bit pointers. */
struct user_i387_struct {
	unsigned short	cwd;
	unsigned short	swd;
	unsigned short	twd;	/* Note this is not the same as
				   the 32bit/x87/FSAVE twd */
	unsigned short	fop;
	uint64_t	rip;
	uint64_t	rdp;
	uint32_t	mxcsr;
	uint32_t	mxcsr_mask;
	uint32_t	st_space[32];	/* 8*16 bytes for each FP-reg = 128 bytes */
	uint32_t	xmm_space[64];	/* 16*16 bytes for each XMM-reg = 256 bytes */
	uint32_t	padding[24];
};

struct user_regs_struct {
	unsigned long	r15;
	unsigned long	r14;
	unsigned long	r13;
	unsigned long	r12;
	unsigned long	bp;
	unsigned long	bx;
	unsigned long	r11;
	unsigned long	r10;
	unsigned long	r9;
	unsigned long	r8;
	unsigned long	ax;
	unsigned long	cx;
	unsigned long	dx;
	unsigned long	si;
	unsigned long	di;
	unsigned long	orig_ax;
	unsigned long	ip;
	unsigned long	cs;
	unsigned long	flags;
	unsigned long	sp;
	unsigned long	ss;
	unsigned long	fs_base;
	unsigned long	gs_base;
	unsigned long	ds;
	unsigned long	es;
	unsigned long	fs;
	unsigned long	gs;
};
/* When the kernel dumps core, it starts by dumping the user struct -
   this will be used by gdb to figure out where the data and stack segments
   are within the file, and what virtual addresses to use. */
struct user {
/* We start with the registers, to mimic the way that "memory" is returned
   from the ptrace(3,...) function.  */
  struct user_regs_struct regs;	/* Where the registers are actually stored */
/* ptrace does not yet supply these.  Someday.... */
  int u_fpvalid;		/* True if math co-processor being used. */
				/* for this mess. Not yet used. */
  int pad0;
  struct user_i387_struct i387;	/* Math Co-processor registers. */
/* The rest of this junk is to help gdb figure out what goes where */
  unsigned long int u_tsize;	/* Text segment size (pages). */
  unsigned long int u_dsize;	/* Data segment size (pages). */
  unsigned long int u_ssize;	/* Stack segment size (pages). */
  unsigned long start_code;     /* Starting virtual address of text. */
  unsigned long start_stack;	/* Starting virtual address of stack area.
				   This is actually the bottom of the stack,
				   the top of the stack is always found in the
				   esp register.  */
  long int signal;		/* Signal that caused the core dump. */
  int reserved;			/* No longer used */
  int pad1;
  unsigned long u_ar0;		/* Used by gdb to help find the values for */
				/* the registers. */
  struct user_i387_struct *u_fpstate;	/* Math Co-processor pointer. */
  unsigned long magic;		/* To uniquely identify a core file */
  char u_comm[32];		/* User command that was responsible */
  unsigned long u_debugreg[8];
  unsigned long error_code; /* CPU error code or 0 */
  unsigned long fault_address; /* CR3 or 0 */
};

uint32_t watch_buffer = 0;

int main(int argc, char const *argv[])
{
    printf("Starting\n");

    // pid_t pid = getpid();

    pid_t child_pid = fork();

    if(child_pid == 0) {
        if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1) {
            printf("Child trace request fail\n");
            abort();
        }
        printf("Child trace enabled and trapping\n");
        __asm__ __volatile__ (
            "int3\n\t"
            :
            :
            :
        );
        printf("Child continue\n");
        watch_buffer = 0;
        printf("Child done\n");
        fflush(stdout);
    } else {

        printf("Parent waiting\n");

        /* parent -- the debugger */
        /* wait for the child to become ready for tracing */
        int ret;
        pid_t waited = waitpid(child_pid, &ret, 0);
        assert(waited == child_pid);
        assert(WIFSTOPPED(ret));
        assert(WSTOPSIG(ret) == SIGTRAP);

        printf("Parent resume\n");

        dr7_t dr7 = {0};

        dr7.dr0_local = 1;
        dr7.le = 1;
        dr7.ge = 1;
        dr7.reserved_10 = 1;

        dr7.dr0_break = 1; /* break on data writes */
        dr7.dr0_len   = 3; /* len 4 */

        uint64_t dr7_value = 0;
        memcpy(&dr7_value, &dr7, sizeof(uint64_t));

    #define DR_OFFSET(n) (void*) (offsetof(struct user, u_debugreg) + n * 8)

        if (ptrace(PTRACE_POKEUSER, child_pid, DR_OFFSET(0), (void*)&watch_buffer) == -1) {
            printf("ptrace1 failed: %s\n", strerror(errno));
            abort();
        }
        printf("Parent ptrace 1 done\n");
        if (ptrace(PTRACE_POKEUSER, child_pid, DR_OFFSET(7), (void*)(uintptr_t)dr7_value) == -1) {
            printf("ptrace2 failed: %s\n", strerror(errno));
            abort();
        }
        printf("Parent ptrace 2 done\n");
        if (ptrace(PTRACE_POKEUSER, child_pid, DR_OFFSET(6), (void*)0) == -1) {
            printf("ptrace3 failed: %s\n", strerror(errno));
            abort();
        }
        printf("Parent ptrace 3 done\n");
        if (ptrace(PTRACE_CONT, child_pid, (void*)0, (void*)0) == -1) {
            printf("ptrace4 failed: %s\n", strerror(errno));
            abort();
        }
        printf("Parent ptrace 4 done\n");

        int status;
        while(1) {
          wait(&status);
          if(WIFEXITED(status)) {
              break;
          } else {
            printf("Parent woken up because buffer written\n");
            if (ptrace(PTRACE_CONT, child_pid, (void*)0, (void*)0) == -1) {
                printf("ptrace5 failed: %s\n", strerror(errno));
                abort();
            }
          }
        }

        printf("Parent done\n");
    }

    return 0;
}
