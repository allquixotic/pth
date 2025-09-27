/*
**  GNU Pth - The GNU Portable Threads
**  Copyright (c) 1999-2006 Ralf S. Engelschall <rse@engelschall.com>
**  Copyright (c) 2025 Modernization - Assembly-based implementation
**
**  pth_mctx.c: Pth machine context handling (Modern Linux - Assembly-based)
**
**  This implementation uses hand-written x86_64 assembly for context switching,
**  replacing the deprecated and unreliable makecontext/swapcontext functions.
**  This approach works with both glibc and musl.
*/

#include "pth_p.h"

extern void pth_mctx_switch_asm(pth_mctx_t *from_mctx, pth_mctx_t *to_mctx);

int pth_mctx_set(
    pth_mctx_t *mctx, void (*func)(void), char *sk_addr_lo, char *sk_addr_hi)
{
    char *stack_top;
    void **stack_ptr;

    if (mctx == NULL || func == NULL || sk_addr_lo == NULL || sk_addr_hi == NULL) {
        return FALSE;
    }

    if (sk_addr_hi <= sk_addr_lo) {
        return FALSE;
    }

    memset(mctx, 0, sizeof(pth_mctx_t));

    stack_top = sk_addr_hi;
    stack_top = (char *)((((uintptr_t)stack_top) & ~0xF) - 8);

    stack_ptr = (void **)stack_top;
    *stack_ptr = NULL;

    mctx->regs[5] = (void *)stack_ptr;
    mctx->regs[4] = (void *)func;

    mctx->start_func = func;

    return TRUE;
}