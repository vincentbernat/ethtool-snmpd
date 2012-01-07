/*
 * Copyright (c) 2012 Vincent Bernat <bernat@luffy.cx>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
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
