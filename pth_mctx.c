/*
**  GNU Pth - The GNU Portable Threads
**  Copyright (c) 1999-2006 Ralf S. Engelschall <rse@engelschall.com>
**
**  This file is part of GNU Pth, a non-preemptive thread scheduling
**  library which can be found at http://www.gnu.org/software/pth/.
**
**  This library is free software; you can redistribute it and/or
**  modify it under the terms of the GNU Lesser General Public
**  License as published by the Free Software Foundation; either
**  version 2.1 of the License, or (at your option) any later version.
**
**  This library is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**  Lesser General Public License for more details.
**
**  You should have received a copy of the GNU Lesser General Public
**  License along with this library; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
**  USA, or contact Ralf S. Engelschall <rse@engelschall.com>.
**
**  pth_mctx.c: Pth machine context handling (Modern Linux - ucontext only)
*/

#include "pth_p.h"

#if cpp

#include <ucontext.h>

typedef struct pth_mctx_st pth_mctx_t;
struct pth_mctx_st {
    ucontext_t uc;
    int restored;
    sigset_t sigs;
    int error;
};

#define pth_mctx_save(mctx) \
        ( (mctx)->error = errno, \
          (mctx)->restored = 0, \
          getcontext(&(mctx)->uc), \
          (mctx)->restored )

#define pth_mctx_restore(mctx) \
        ( errno = (mctx)->error, \
          (mctx)->restored = 1, \
          (void)setcontext(&(mctx)->uc) )

#define pth_mctx_restored(mctx) \
        /*nop*/

#define SWITCH_DEBUG_LINE \
        "==== THREAD CONTEXT SWITCH ==========================================="
#ifdef PTH_DEBUG
#define  _pth_mctx_switch_debug pth_debug(NULL, 0, 1, SWITCH_DEBUG_LINE);
#else
#define  _pth_mctx_switch_debug /*NOP*/
#endif

#define pth_mctx_switch(old,new) \
    _pth_mctx_switch_debug \
    swapcontext(&((old)->uc), &((new)->uc));

#endif

int pth_mctx_set(
    pth_mctx_t *mctx, void (*func)(void), char *sk_addr_lo, char *sk_addr_hi)
{
    if (getcontext(&(mctx->uc)) != 0)
        return FALSE;

    mctx->uc.uc_link           = NULL;

    mctx->uc.uc_stack.ss_sp    = pth_skaddr(makecontext, sk_addr_lo, sk_addr_hi-sk_addr_lo);
    mctx->uc.uc_stack.ss_size  = pth_sksize(makecontext, sk_addr_lo, sk_addr_hi-sk_addr_lo);
    mctx->uc.uc_stack.ss_flags = 0;

    makecontext(&(mctx->uc), func, 0+1);

    return TRUE;
}