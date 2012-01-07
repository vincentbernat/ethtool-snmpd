#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

#######################
### Base configuration

# Configure autoconf
AC_PREREQ([2.64])
AC_INIT([ethtool-snmpd], [0.1], [bernat@luffy.cx])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_CONFIG_MACRO_DIR([m4])

# Configure automake
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AM_MAINTAINER_MODE
AM_SILENT_RULES(yes)

# Configure libtool
LT_PREREQ([2.2])
LT_INIT

#######################
### Checks

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O

# Check some compiler flags
AX_CFLAGS_GCC_OPTION([-fdiagnostics-show-option])
AX_CFLAGS_GCC_OPTION([-std=gnu99])
AX_CFLAGS_GCC_OPTION([-pipe])
AX_CFLAGS_GCC_OPTION([-Wall])
AX_CFLAGS_GCC_OPTION([-W])
AX_CFLAGS_GCC_OPTION([-Wextra])
AX_CFLAGS_GCC_OPTION([-Wformat])
AX_CFLAGS_GCC_OPTION([-Wformat-security])
AX_CFLAGS_GCC_OPTION([-Wcast-align])
AX_CFLAGS_GCC_OPTION([-Winline])
AX_CFLAGS_GCC_OPTION([-fstack-protector])
AX_CFLAGS_GCC_OPTION([-D_FORTIFY_SOURCE=2])

AC_CACHE_SAVE

# Check for ethtool stuff
ethtool_snmpd_CHECK_HEADER_ETHTOOL
ethtool_snmpd_CHECK_SNMP

AC_CACHE_SAVE
AC_OUTPUT

cat <<EOF

------------------ Summary ------------------
 $PACKAGE_NAME version $PACKAGE_VERSION
  Prefix.........: $prefix
  C Compiler.....: $CC $CFLAGS $CPPFLAGS
  Linker.........: $LD $LDFLAGS $LIBS
---------------------------------------------

Check the above options and compile with:
 ${MAKE-make}

EOF
