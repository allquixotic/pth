AC_PREREQ([2.69])
AC_INIT([gnu-pth], [2.0.8], [bug-pth@gnu.org])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_SRCDIR([pth_lib.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB
LT_INIT

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([gettimeofday memset select socket])

# Machine Context Method
AC_CHECK_HEADER([ucontext.h], [have_ucontext_h=yes], [have_ucontext_h=no])
AC_CHECK_FUNCS([getcontext makecontext swapcontext])

AC_MSG_CHECKING([for machine context method])
if test "$ac_cv_func_getcontext" = yes -a "$ac_cv_func_makecontext" = yes -a "$ac_cv_func_swapcontext" = yes -a "$have_ucontext_h" = yes; then
    mctx_mth="mcsc"
    AC_DEFINE([HAVE_MCSC], [1], [Define to 1 if you have the MCSC functions.])
    AC_MSG_RESULT([mcsc (makecontext/swapcontext)])
else
    mctx_mth="sjlj"
    AC_DEFINE([HAVE_SJLJ], [1], [Define to 1 if you have the SJLJ functions.])
    AC_MSG_RESULT([sjlj (setjmp/longjmp)])
fi

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
