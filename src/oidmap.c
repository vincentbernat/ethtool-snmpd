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

#include "ethtool-snmpd.h"
#include "tree.h"

/* Implementation using red/black trees. */

struct oidmap_node {
  RB_ENTRY(oidmap_node) entry;
  struct oidmap_entry   om;
};
static int
oidmap_node_cmp(struct oidmap_node *e1, struct oidmap_node *e2) {
  return snmp_oid_compare(e1->om.oid, e1->om.oid_len,
			  e2->om.oid, e2->om.oid_len);
}
RB_HEAD(oidmap, oidmap_node);
RB_GENERATE(oidmap, oidmap_node, entry, oidmap_node_cmp);

/* New OID map */
struct oidmap*
oidmap_new() {
  return (struct oidmap*)calloc(1, sizeof(struct oidmap));
}

/* Insert a new value in the map */
struct oidmap_entry*
oidmap_insert(struct oidmap *map,
	      oid *oid, size_t oid_len,
	      u_int64_t value) {
  struct oidmap_node *n = calloc(1, sizeof(struct oidmap_node) + sizeof(oid) * oid_len);
  if (!n) {
    log_warn("not enough memory to allocate a new node");
    return NULL;
  }
  memcpy(n->om.oid, oid, oid_len * sizeof(oid));
  n->om.oid_len = oid_len;
  n->om.value = value;
  if (RB_INSERT(oidmap, map, n) != NULL) {
    /* Entry already present. */
    free(n);
    return NULL;
  }
  return &n->om;
}

/* Free an OID map. */
void
oidmap_free(struct oidmap *map) {
  struct oidmap_node *current, *next;
  for (current = RB_MIN(oidmap, map); current; current = next) {
    next = RB_NEXT(oidmap, map, current);
    RB_REMOVE(oidmap, map, current);
    free(current);
  }
  free(map);
}

static struct oidmap_node*
oidmap_build_node(oid *oid, size_t oid_len) {
  static struct oidmap_node *n = NULL;
  if (!n) n = calloc(1, sizeof(struct oidmap_node) + sizeof(oid) * MAX_OID_LEN);
  if (!n) {
    log_warn("not enough memory to allocate a new node");
    return NULL;
  }
  memcpy(n->om.oid, oid, oid_len * sizeof(oid));
  n->om.oid_len = oid_len;
  return n;
}

/* Search an entry */
struct oidmap_entry*
oidmap_search(struct oidmap *map,
	      oid *oid, size_t oid_len) {
  struct oidmap_node *n = oidmap_build_node(oid, oid_len);
  if (!n) return NULL;
  struct oidmap_node *result = RB_FIND(oidmap, map, n);
  if (!result) return NULL;
  return &result->om;
}

/* Search the next entry */
struct oidmap_entry*
oidmap_search_next(struct oidmap *map,
		   oid *oid, size_t oid_len) {
  struct oidmap_node *n = oidmap_build_node(oid, oid_len);
  if (!n) return NULL;
  struct oidmap_node *result = RB_NFIND(oidmap, map, n);
  if (!result) return NULL;
  if (snmp_oid_compare(oid, oid_len, result->om.oid, result->om.oid_len) == 0)
    result = RB_NEXT(oidmap, map, result);
  if (!result) return NULL;
  return &result->om;
}
