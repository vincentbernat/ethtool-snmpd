/*
 * Copyright (c) 2012 Vincent Bernat <bernat@luffy.cx>
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef ETHTOOL_SNMPD_H
#define ETHTOOL_SNMPD_H

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

/* log.c */
void log_init(int, const char *);
void log_warn(const char *, ...) __attribute__ ((format (printf, 1, 2)));
void log_warnx(const char *, ...) __attribute__ ((format (printf, 1, 2)));
void log_info(const char *, ...) __attribute__ ((format (printf, 1, 2)));
void log_debug(const char *, ...) __attribute__ ((format (printf, 1, 2)));
void fatal(const char *);
void fatalx(const char *);

/* ethtool-stats.c */
void init_ethtoolStatTable(void);

/* oidmap.c */
struct oidmap;
struct oidmap_entry {
  u_int64_t value;
  size_t    oid_len;
  oid       oid[0];
};
struct oidmap       *oidmap_new(void);
struct oidmap_entry *oidmap_insert(struct oidmap*,
				   oid*, size_t, u_int64_t);
void                 oidmap_free(struct oidmap*);
struct oidmap_entry *oidmap_search(struct oidmap*,
				   oid*, size_t);
struct oidmap_entry *oidmap_search_next(struct oidmap*,
					oid*, size_t);

#endif
