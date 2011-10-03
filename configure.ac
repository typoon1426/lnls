#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([lnls], [0.1], [cucchi@cs.unibo.it])
AM_INIT_AUTOMAKE(lnls, 1.0)
AC_CONFIG_SRCDIR([include/logging.h])
AC_CONFIG_SRCDIR([src/logging.c])
AC_CONFIG_HEADERS([config.h])

: ${CFLAGS=""}

# Checks for programs.
AC_PROG_CC([gcc cc])

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([net/if.h netdb.h arpa/inet.h fcntl.h netdb.h stdlib.h string.h sys/socket.h sys/time.h sys/stat.h syslog.h unistd.h syslog.h getopt.h errno.h fcntl.h time.h linux/netlink.h linux/rtnetlink.h netinet/ether.h sys/socket.h signal.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_STRNLEN
AC_CHECK_FUNCS([getcwd memset memcpy socket])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
#AM_PROG_CC_C_O()
AC_OUTPUT
