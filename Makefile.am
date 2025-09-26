AUTOMAKE_OPTIONS = foreign

lib_LTLIBRARIES = libpth.la
libpth_la_SOURCES = \
pth_acmac.h \
pth_p.h \
pth_acdef.h \
pth_attr.c \
pth_cancel.c \
pth_clean.c \
pth_compat.c \
pth_data.c \
pth_debug.c \
pth_errno.c \
pth_event.c \
pth_ext.c \
pth_fork.c \
pth_high.c \
pth_lib.c \
pth_mctx.c \
pth_msg.c \
pth_pqueue.c \
pth_ring.c \
pth_sched.c \
pth_string.c \
pth_sync.c \
pth_syscall.c \
pth_tcb.c \
pth_time.c \
pth_uctx.c \
pth_util.c \
pth_vers.c \
pth.h

libpth_la_LDFLAGS = -version-info 7:0:0

include_HEADERS = pth.h

bin_PROGRAMS = test_std test_mp test_misc test_philo test_sig test_select test_httpd test_sfio test_uctx test_pthread

test_std_SOURCES = test_std.c test_common.c
test_std_LDADD = libpth.la

test_mp_SOURCES = test_mp.c test_common.c
test_mp_LDADD = libpth.la

test_misc_SOURCES = test_misc.c
test_misc_LDADD = libpth.la

test_philo_SOURCES = test_philo.c test_common.c
test_philo_LDADD = libpth.la

test_sig_SOURCES = test_sig.c
test_sig_LDADD = libpth.la


test_select_SOURCES = test_select.c
test_select_LDADD = libpth.la

test_httpd_SOURCES = test_httpd.c test_common.c
test_httpd_LDADD = libpth.la

test_sfio_SOURCES = test_sfio.c
test_sfio_LDADD = libpth.la

test_uctx_SOURCES = test_uctx.c
test_uctx_LDADD = libpth.la

test_pthread_SOURCES = test_pthread.c
test_pthread_LDADD = libpth.la

