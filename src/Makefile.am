AM_CFLAGS = $(MORE_CFLAGS)
AM_CPPFLAGS = $(MORE_CPPFLAGS)
AM_LDFLAGS = $(MORE_LDFLAGS)

bin_PROGRAMS = ethtool-snmpd
ethtool_snmpd_SOURCES = main.c ethtool-stats.c log.c oidmap.c ethtool-snmpd.h tree.h
ethtool_snmpd_CFLAGS = $(AM_CFLAGS) @NETSNMP_CFLAGS@
ethtool_snmpd_LDFLAGS = $(AM_LDFLAGS) @NETSNMP_LIBS@
