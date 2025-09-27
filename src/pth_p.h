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
**  pth_p.h: Pth private API definitions
*/

#ifndef _PTH_P_H_
#define _PTH_P_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <time.h>

#define _PTH_VERS_C_AS_HEADER_
#include "pth_vers.c"
#undef  _PTH_VERS_C_AS_HEADER_

#define _PTH_PRIVATE
#include "pth.h"
#undef _PTH_PRIVATE

#include "pth_acdef.h"
#include "pth_acmac.h"

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#ifdef HAVE_NET_ERRNO_H
#include <net/errno.h>
#endif
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif

#ifdef PTH_DMALLOC
#include <dmalloc.h>
#endif

#ifdef PTH_EX
#define __EX_NS_USE_CUSTOM__
#include "ex.h"
#endif

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif
#ifdef _PATH_BSHELL
#define PTH_PATH_BINSH _PATH_BSHELL
#else
#define PTH_PATH_BINSH "/bin/sh"
#endif

#define O_NONBLOCKING O_NONBLOCK

#define COMPILER_HAPPYNESS(name) \
    int __##name##_unit = 0;

#define PTH_TCB_NAMELEN 40

enum {
    PTH_ATTR_GET,
    PTH_ATTR_SET
};

struct pth_attr_st {
    pth_t        a_tid;
    int          a_prio;
    int          a_dispatches;
    char         a_name[PTH_TCB_NAMELEN];
    int          a_joinable;
    unsigned int a_cancelstate;
    unsigned int a_stacksize;
    char        *a_stackaddr;
};

typedef struct pth_cleanup_st pth_cleanup_t;
struct pth_cleanup_st {
    pth_cleanup_t *next;
    void (*func)(void *);
    void *arg;
};

typedef int (*pth_event_func_t)(void *);

struct pth_event_st {
    struct pth_event_st *ev_next;
    struct pth_event_st *ev_prev;
    pth_status_t ev_status;
    int ev_type;
    int ev_goal;
    union {
        struct { int fd; }                                          FD;
        struct { int *n; int nfd; fd_set *rfds, *wfds, *efds; }     SELECT;
        struct { sigset_t *sigs; int *sig; }                        SIGS;
        struct { pth_time_t tv; }                                   TIME;
        struct { pth_msgport_t mp; }                                MSG;
        struct { pth_mutex_t *mutex; }                              MUTEX;
        struct { pth_cond_t *cond; }                                COND;
        struct { pth_t tid; }                                       TID;
        struct { pth_event_func_t func; void *arg; pth_time_t tv; } FUNC;
    } ev_args;
};

typedef struct pth_mctx_st pth_mctx_t;
struct pth_mctx_st {
    void *regs[9];
    void (*start_func)(void);
    sigset_t sigs;
    int error;
};

struct pth_msgport_st {
    pth_ringnode_t mp_node;
    const char    *mp_name;
    pth_t          mp_tid;
    pth_ring_t     mp_queue;
};

struct pth_pqueue_st {
    pth_t q_head;
    int   q_num;
};
typedef struct pth_pqueue_st pth_pqueue_t;

struct pth_st {
    pth_t          q_next;
    pth_t          q_prev;
    int            q_prio;

    int            prio;
    char           name[PTH_TCB_NAMELEN];
    int            dispatches;
    pth_state_t    state;

    pth_time_t     spawned;
    pth_time_t     lastran;
    pth_time_t     running;

    pth_event_t    events;

    sigset_t       sigpending;
    int            sigpendcnt;

    pth_mctx_t     mctx;
    char          *stack;
    unsigned int   stacksize;
    long          *stackguard;
    int            stackloan;
    void        *(*start_func)(void *);
    void          *start_arg;

    int            joinable;
    void          *join_arg;

    const void   **data_value;
    int            data_count;

    int            cancelreq;
    unsigned int   cancelstate;
    pth_cleanup_t *cleanups;

    pth_ring_t     mutexring;

#ifdef PTH_EX
    ex_ctx_t       ex_ctx;
#endif
};

extern int pth_initialized;
extern int pth_errno_storage;
extern int pth_errno_flag;

extern pth_t        pth_main;
extern pth_t        pth_sched;
extern pth_t        pth_current;
extern pth_pqueue_t pth_NQ;
extern pth_pqueue_t pth_RQ;
extern pth_pqueue_t pth_WQ;
extern pth_pqueue_t pth_SQ;
extern pth_pqueue_t pth_DQ;
extern int          pth_favournew;
extern float        pth_loadval;
extern pth_time_t   pth_time_zero;

#if PTH_SYSCALL_SOFT
#define pth_sc(func) pth_sc_##func
#else
#define pth_sc(func) func
#endif

extern int pth_snprintf(char *str, size_t count, const char *fmt, ...);
extern int pth_vsnprintf(char *str, size_t count, const char *fmt, va_list args);
extern pth_t pth_tcb_alloc(unsigned int stacksize, void *stackaddr);
extern void pth_tcb_free(pth_t t);
extern int pth_mctx_set(pth_mctx_t *mctx, void (*func)(void), char *sk_addr_lo, char *sk_addr_hi);
extern int pth_time_cmp(pth_time_t *t1, pth_time_t *t2);
extern double pth_time_t2d(pth_time_t *t);
extern void pth_mutex_releaseall(pth_t thread);
extern int pth_util_sigdelete(int sig);
extern int pth_attr_ctrl(int cmd, pth_attr_t a, int op, va_list ap);
extern void pth_cleanup_popall(pth_t t, int execute);
extern void pth_key_destroydata(pth_t t);
extern void pth_debug(const char *file, int line, int argc, const char *fmt, ...);
extern void pth_dumpstate(FILE *fp);
extern void pth_dumpqueue(FILE *fp, const char *qn, pth_pqueue_t *q);
extern ssize_t pth_readv_faked(int fd, const struct iovec *iov, int iovcnt);
extern ssize_t pth_writev_iov_bytes(const struct iovec *iov, int iovcnt);
extern void pth_writev_iov_advance(const struct iovec *riov, int riovcnt, size_t advance, struct iovec **wiov, int *wiovcnt, struct iovec *tiov, int tiovcnt);
extern ssize_t pth_writev_faked(int fd, const struct iovec *iov, int iovcnt);
extern int pth_thread_exists(pth_t t);
extern void pth_thread_cleanup(pth_t thread);
extern void pth_pqueue_init(pth_pqueue_t *q);
extern void pth_pqueue_insert(pth_pqueue_t *q, int prio, pth_t t);
extern pth_t pth_pqueue_delmax(pth_pqueue_t *q);
extern void pth_pqueue_delete(pth_pqueue_t *q, pth_t t);
extern int pth_pqueue_favorite(pth_pqueue_t *q, pth_t t);
extern void pth_pqueue_increase(pth_pqueue_t *q);
extern pth_t pth_pqueue_tail(pth_pqueue_t *q);
extern pth_t pth_pqueue_walk(pth_pqueue_t *q, pth_t t, int direction);
extern int pth_pqueue_contains(pth_pqueue_t *q, pth_t t);
extern void pth_ring_init(pth_ring_t *r);
extern void pth_ring_insert_after(pth_ring_t *r, pth_ringnode_t *rn1, pth_ringnode_t *rn2);
extern void pth_ring_insert_before(pth_ring_t *r, pth_ringnode_t *rn1, pth_ringnode_t *rn2);
extern void pth_ring_delete(pth_ring_t *r, pth_ringnode_t *rn);
extern void pth_ring_prepend(pth_ring_t *r, pth_ringnode_t *rn);
extern void pth_ring_append(pth_ring_t *r, pth_ringnode_t *rn);
extern pth_ringnode_t *pth_ring_pop(pth_ring_t *r);
extern int pth_ring_favorite(pth_ring_t *r, pth_ringnode_t *rn);
extern pth_ringnode_t *pth_ring_dequeue(pth_ring_t *r);
extern int pth_ring_contains(pth_ring_t *r, pth_ringnode_t *rns);
extern int pth_scheduler_init(void);
extern void pth_scheduler_drop(void);
extern void pth_scheduler_kill(void);
extern void *pth_scheduler(void *);
extern void pth_sched_eventmanager(pth_time_t *now, int dopoll);
extern char *pth_util_cpystrn(char *dst, const char *src, size_t dst_size);
extern int pth_util_fd_valid(int fd);
extern void pth_util_fds_merge(int nfd, fd_set *ifds1, fd_set *ofds1, fd_set *ifds2, fd_set *ofds2, fd_set *ifds3, fd_set *ofds3);
extern int pth_util_fds_test(int nfd, fd_set *ifds1, fd_set *ofds1, fd_set *ifds2, fd_set *ofds2, fd_set *ifds3, fd_set *ofds3);
extern int pth_util_fds_select(int nfd, fd_set *ifds1, fd_set *ofds1, fd_set *ifds2, fd_set *ofds2, fd_set *ifds3, fd_set *ofds3);
extern void pth_syscall_init(void);
extern void pth_syscall_kill(void);

extern void pth_mctx_switch_asm(pth_mctx_t *from_mctx, pth_mctx_t *to_mctx);

#define pth_mctx_save(mctx) \
        ( (mctx)->error = errno, 0 )

#define pth_mctx_restore(mctx) \
        ( errno = (mctx)->error, (void)0 )

#define pth_mctx_restored(mctx) \
        ((void)0)

#ifndef PTH_DEBUG

#define pth_debug1(a1)
#define pth_debug2(a1, a2)
#define pth_debug3(a1, a2, a3)
#define pth_debug4(a1, a2, a3, a4)
#define pth_debug5(a1, a2, a3, a4, a5)
#define pth_debug6(a1, a2, a3, a4, a5, a6)

#else

#define pth_debug1(a1)                     pth_debug(__FILE__, __LINE__, 1, a1)
#define pth_debug2(a1, a2)                 pth_debug(__FILE__, __LINE__, 2, a1, a2)
#define pth_debug3(a1, a2, a3)             pth_debug(__FILE__, __LINE__, 3, a1, a2, a3)
#define pth_debug4(a1, a2, a3, a4)         pth_debug(__FILE__, __LINE__, 4, a1, a2, a3, a4)
#define pth_debug5(a1, a2, a3, a4, a5)     pth_debug(__FILE__, __LINE__, 5, a1, a2, a3, a4, a5)
#define pth_debug6(a1, a2, a3, a4, a5, a6) pth_debug(__FILE__, __LINE__, 6, a1, a2, a3, a4, a5, a6)

#endif

#define pth_shield \
        for ( pth_errno_storage = errno, \
              pth_errno_flag = TRUE; \
              pth_errno_flag; \
              errno = pth_errno_storage, \
              pth_errno_flag = FALSE )

#if defined(PTH_DEBUG)
#define pth_error(return_val,errno_val) \
        (errno = (errno_val), \
        pth_debug4("return 0x%lx with errno %d(\"%s\")", \
                   (unsigned long)(return_val), (errno), strerror((errno))), \
        (return_val))
#else
#define pth_error(return_val,errno_val) \
        (errno = (errno_val), (return_val))
#endif

#define pth_implicit_init() \
    if (!pth_initialized) \
        pth_init()

#define pth_pqueue_favorite_prio(q) \
    ((q)->q_head != NULL ? (q)->q_head->q_prio + 1 : PTH_PRIO_MAX)

#define pth_pqueue_elements(q) \
    ((q) == NULL ? (-1) : (q)->q_num)

#define pth_pqueue_head(q) \
    ((q) == NULL ? NULL : (q)->q_head)

#define pth_ring_elements(r) \
    ({ pth_ring_t *_r = (r); \
       int _result; \
       if (_r == NULL) \
           _result = -1; \
       else \
           _result = (int)(_r->r_nodes); \
       _result; })

#define pth_ring_first(r) \
    ((r) == NULL ? NULL : (r)->r_hook)

#define pth_ring_last(r) \
    ((r) == NULL ? NULL : ((r)->r_hook == NULL ? NULL : (r)->r_hook->rn_prev))

#define pth_ring_next(r, rn) \
    (((r) == NULL || (rn) == NULL) ? NULL : ((rn)->rn_next == (r)->r_hook ? NULL : (rn)->rn_next))

#define pth_ring_prev(r, rn) \
    (((r) == NULL || (rn) == NULL) ? NULL : ((rn)->rn_prev == (r)->r_hook->rn_prev ? NULL : (rn)->rn_prev))

#define pth_ring_insert(r, rn) \
    pth_ring_append((r), (rn))

#define pth_ring_push(r, rn) \
    pth_ring_prepend((r), (rn))

#define pth_ring_enqueue(r, rn) \
    pth_ring_prepend((r), (rn))

#define PTH_TIME_NOW  (pth_time_t *)(0)
#define PTH_TIME_ZERO &pth_time_zero
#define PTH_TIME(sec,usec) { sec, usec }

#define pth_time_equal(t1,t2) \
        (((t1).tv_sec == (t2).tv_sec) && ((t1).tv_usec == (t2).tv_usec))

#if defined(HAVE_GETTIMEOFDAY_ARGS1)
#define __gettimeofday(t) gettimeofday(t)
#else
#define __gettimeofday(t) gettimeofday(t, NULL)
#endif
#define pth_time_set(t1,t2) \
    do { \
        if ((t2) == PTH_TIME_NOW) \
            __gettimeofday((t1)); \
        else { \
            (t1)->tv_sec  = (t2)->tv_sec; \
            (t1)->tv_usec = (t2)->tv_usec; \
        } \
    } while (0)

#define pth_time_add(t1,t2) \
    (t1)->tv_sec  += (t2)->tv_sec; \
    (t1)->tv_usec += (t2)->tv_usec; \
    if ((t1)->tv_usec > 1000000) { \
        (t1)->tv_sec  += 1; \
        (t1)->tv_usec -= 1000000; \
    }

#define pth_time_sub(t1,t2) \
    (t1)->tv_sec  -= (t2)->tv_sec; \
    (t1)->tv_usec -= (t2)->tv_usec; \
    if ((t1)->tv_usec < 0) { \
        (t1)->tv_sec  -= 1; \
        (t1)->tv_usec += 1000000; \
    }

#define pth_util_min(a,b) \
        ((a) > (b) ? (b) : (a))

/* machine context switch macro */
#define SWITCH_DEBUG_LINE \
        "==== THREAD CONTEXT SWITCH ==========================================="
#ifdef PTH_DEBUG
#define  _pth_mctx_switch_debug pth_debug(NULL, 0, 1, SWITCH_DEBUG_LINE);
#else
#define  _pth_mctx_switch_debug /*NOP*/
#endif

#define pth_mctx_switch(old,new) \
    do { \
        _pth_mctx_switch_debug \
        (old)->error = errno; \
        pth_mctx_switch_asm((old), (new)); \
        errno = (old)->error; \
    } while(0)

#endif