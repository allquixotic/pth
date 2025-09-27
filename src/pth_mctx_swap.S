/*
**  GNU Pth - The GNU Portable Threads
**  Copyright (c) 1999-2006 Ralf S. Engelschall <rse@engelschall.com>
**  Copyright (c) 2025 Modernization - Assembly-based context switching
**
**  Context switching implementation using x86_64 assembly
**  Inspired by libaco (Apache 2.0 licensed)
**
**  This replaces the deprecated makecontext/swapcontext functions
**  and provides a fast, portable, musl-compatible implementation.
*/

.text
.globl pth_mctx_switch_asm
.type  pth_mctx_switch_asm, @function
.intel_syntax noprefix

/*
    extern void pth_mctx_switch_asm(pth_mctx_t* from_mctx, pth_mctx_t* to_mctx);

    struct pth_mctx_st {
        void*  regs[9];   // r12, r13, r14, r15, rip, rsp, rbx, rbp, fpucw_mxcsr
        int    restored;
        sigset_t sigs;
        int    error;
    };

    Register layout in regs[] array (x86_64):
    [0] = r12
    [1] = r13
    [2] = r14
    [3] = r15
    [4] = rip (return address)
    [5] = rsp (stack pointer)
    [6] = rbx
    [7] = rbp (frame pointer)
    [8] = fpucw_mxcsr (16-bit FPU control word + 32-bit MXCSR)
*/

pth_mctx_switch_asm:
#ifdef __x86_64__
    mov     rdx, QWORD PTR [rsp]      // get return address
    lea     rcx, [rsp+0x8]             // get rsp (after return addr is popped)

    // Save from_mctx registers (rdi = from_mctx)
    mov     QWORD PTR [rdi+0x0],  r12
    mov     QWORD PTR [rdi+0x8],  r13
    mov     QWORD PTR [rdi+0x10], r14
    mov     QWORD PTR [rdi+0x18], r15
    mov     QWORD PTR [rdi+0x20], rdx  // save return address
    mov     QWORD PTR [rdi+0x28], rcx  // save rsp
    mov     QWORD PTR [rdi+0x30], rbx
    mov     QWORD PTR [rdi+0x38], rbp

    // Save FPU control word and MXCSR
    fnstcw  WORD  PTR [rdi+0x40]
    stmxcsr DWORD PTR [rdi+0x44]

    // Restore to_mctx registers (rsi = to_mctx)
    mov     r12, QWORD PTR [rsi+0x0]
    mov     r13, QWORD PTR [rsi+0x8]
    mov     r14, QWORD PTR [rsi+0x10]
    mov     r15, QWORD PTR [rsi+0x18]
    mov     rax, QWORD PTR [rsi+0x20]  // get return address
    mov     rcx, QWORD PTR [rsi+0x28]  // get rsp
    mov     rbx, QWORD PTR [rsi+0x30]
    mov     rbp, QWORD PTR [rsi+0x38]

    // Restore FPU control word and MXCSR
    fldcw   WORD  PTR     [rsi+0x40]
    ldmxcsr DWORD PTR     [rsi+0x44]

    // Switch stack and jump to return address
    mov     rsp, rcx
    jmp     rax
#else
    #error "Only x86_64 is supported"
#endif

.size pth_mctx_switch_asm, .-pth_mctx_switch_asm