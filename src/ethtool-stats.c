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

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/agent_sysORTable.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <linux/sockios.h>
#include <linux/ethtool.h>

#include "ethtool-snmpd.h"

/* Our cache */
#define CACHE_TIMEOUT 30
struct oidmap    *cache_nic_stats   = NULL;
time_t            cache_nic_updated = 0;
struct oidmap    *cache_phy_stats   = NULL;
time_t            cache_phy_updated = 0;

/* Add a value to the cache */
static int
cache(struct oidmap *map,
      unsigned int ifindex, char *name, uint64_t value) {
  oid    o[MAX_OID_LEN];
  size_t len = strlen(name), i;
  o[0] = ifindex;
  for (i = 0; i < len && i < MAX_OID_LEN - 1; i++)
    o[i+1] = name[i];
  return (oidmap_insert(map, o, len+1, value) != NULL);
}

static void
refresh_cache(struct oidmap **cache_stats, time_t *cache_updated,
              int cmd, int stringset, const char *name) {
  /* Do we really need a refresh? */
  struct oidmap *new_cache = NULL;
  time_t now = time(NULL);
  if (*cache_stats &&
      (now - *cache_updated < CACHE_TIMEOUT) &&
      (now >= *cache_updated))
    return;

  int skfd = -1;		/* Socket for ioctl */
  struct ifaddrs *ifap = NULL, *ifa, *ifaprev;
  if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    log_warn("Unable to open socket for ioctl");
    goto endrefresh;
  }

  if (getifaddrs(&ifap) != 0) {
    log_warn("Unable to get list of available interfaces");
    goto endrefresh;
  }
  if ((new_cache = oidmap_new()) == NULL) {
    log_warnx("Unable to allocate memory for cache");
    goto endrefresh;
  }

  /* Check stats for each interface */
  for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
    struct ethtool_drvinfo drvinfo;
    struct ethtool_gstrings *strings = NULL;
    struct ethtool_stats    *stats   = NULL;
    struct ifreq ifr;
    unsigned int n_stats, sz_str, sz_stats, i, ifindex;

    /* We want to process interfaces only once. Check if this one was
       already processed. */
    for (ifaprev = ifap;
	 ifaprev && ifaprev != ifa;
	 ifaprev = ifaprev->ifa_next)
      if (strcmp(ifaprev->ifa_name, ifa->ifa_name) == 0) goto nextif;

    log_debug("grab %s statistics for %s", name, ifa->ifa_name);

    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, ifa->ifa_name);

    /* How many statistics are available ? */
    drvinfo.cmd = ETHTOOL_GDRVINFO;
    ifr.ifr_data = (caddr_t) &drvinfo;
    if (ioctl(skfd, SIOCETHTOOL, &ifr) != 0) {
      log_debug("unable to get driver info for %s", ifa->ifa_name);
      continue;
    }
    n_stats = drvinfo.n_stats;
    if (n_stats < 1) {
      log_debug("no %s statistics available for %s", name, ifa->ifa_name);
      continue;
    }

    /* Allocate memory to grab stat names and values */
    sz_str = n_stats * ETH_GSTRING_LEN;
    sz_stats = n_stats * sizeof(uint64_t);
    strings = calloc(1, sz_str + sizeof(struct ethtool_gstrings));
    if (!strings) {
      log_warnx("unable to allocate memory for strings");
      goto nextif;
    }
    stats = calloc(1, sz_stats + sizeof(struct ethtool_stats));
    if (!stats) {
      log_warnx("unable to allocate memory for stats");
      goto nextif;
    }

    /* Grab stat names */
    strings->cmd = ETHTOOL_GSTRINGS;
    strings->string_set = stringset;
    strings->len = n_stats;
    ifr.ifr_data = (caddr_t) strings;
    if (ioctl(skfd, SIOCETHTOOL, &ifr) != 0) {
      log_warn("unable to get %s statistic names for %s", name, ifa->ifa_name);
      goto nextif;
    }

    /* Grab stat values */
    stats->cmd = cmd;
    stats->n_stats = n_stats;
    ifr.ifr_data = (caddr_t) stats;
    if (ioctl(skfd, SIOCETHTOOL, &ifr) != 0) {
      log_warn("unable to get %s statistic values for %s", name, ifa->ifa_name);
      goto nextif;
    }

    ifindex = if_nametoindex(ifa->ifa_name);
    for (i = 0; i < n_stats; i++)
      if (!cache(new_cache,
		 ifindex,
		 (char *)&strings->data[i * ETH_GSTRING_LEN],
		 stats->data[i])) {
	/* Not able to add to cache, already here, skip to next if. */
	log_debug("not able to add entry for %s, interface already processed?",
		  ifa->ifa_name);
	break;
      }

  nextif:
    if (stats) free(stats);
    if (strings) free(strings);
  }

  /* Replace the cache with the new cache */
  if (*cache_stats) oidmap_free(*cache_stats);
  *cache_stats = new_cache;
  *cache_updated = now;
 endrefresh:
  if (*cache_stats != new_cache && new_cache) oidmap_free(new_cache);
  if (ifap) freeifaddrs(ifap);
  if (skfd >= 0) close(skfd);
}

static void
refresh_nic_cache(void) {
  refresh_cache(&cache_nic_stats, &cache_nic_updated,
                ETHTOOL_GSTATS, ETH_SS_STATS, "NIC");
}

static void
refresh_phy_cache(void) {
#ifdef ETHTOOL_GPHYSTATS
  refresh_cache(&cache_phy_stats, &cache_phy_updated,
                ETHTOOL_GPHYSTATS, ETH_SS_PHY_STATS, "PHY");
#endif
}

/* Handle an incoming request for ethtool*StatTable */
static u_char*
ethtool_stat(struct variable *vp, oid *name, size_t *length,
	     int exact, size_t *var_len, WriteMethod **write_method,
             struct oidmap **cache_stats,
             void (*refresh_cache)(void)) {
  static struct counter64 counter64_ret;

  *write_method = NULL;

  /* Start with a known prefix */
  if (snmp_oid_compare(name, *length, vp->name, vp->namelen) < 0) {
    if (exact) return NULL;
    memcpy(name, vp->name, sizeof(oid) * vp->namelen);
    *length = vp->namelen;
  }

  /* Refresh cache */
  refresh_cache();
  if (!*cache_stats) return NULL;

  /* Search a matching node */
  struct oidmap_entry *e;
  if (exact) {
    e = oidmap_search(*cache_stats,
		      name + vp->namelen,
		      *length - vp->namelen);
    if (!e) return NULL;
  } else {
    e = oidmap_search_next(*cache_stats,
			   name + vp->namelen,
			   *length - vp->namelen);
    if (!e) return NULL;
    memcpy(name + vp->namelen, e->oid, e->oid_len * sizeof(oid));
    *length = vp->namelen + e->oid_len;
  }
 
  counter64_ret.low = e->value & 0xffffffff;
  counter64_ret.high = e->value >> 32;
  *var_len = sizeof(struct counter64);
  return (u_char *)&counter64_ret;
}

static u_char*
ethtool_nic_stat(struct variable *vp, oid *name, size_t *length,
                 int exact, size_t *var_len, WriteMethod **write_method) {
  return ethtool_stat(vp, name, length, exact, var_len, write_method,
                      &cache_nic_stats, refresh_nic_cache);
}

static u_char*
ethtool_phy_stat(struct variable *vp, oid *name, size_t *length,
                 int exact, size_t *var_len, WriteMethod **write_method) {
  return ethtool_stat(vp, name, length, exact, var_len, write_method,
                      &cache_phy_stats, refresh_phy_cache);
}

static oid ethtool_oid[] = {1, 3, 6, 1, 4, 1, 39178, 100, 1};
static struct variable3 ethtool_vars[] = {
  {1, ASN_COUNTER64, RONLY, ethtool_nic_stat, 3, {1, 1, 2}},
  {2, ASN_COUNTER64, RONLY, ethtool_phy_stat, 3, {2, 1, 2}},
};

void
init_ethtoolStatTable() {
  int rc;
  log_debug("Initializing ethtoolStatTable");
  REGISTER_MIB("ethtoolStatTable", ethtool_vars, variable3, ethtool_oid);
  if ((rc = register_sysORTable(ethtool_oid, OID_LENGTH(ethtool_oid),
                                "The MIB module for ethtool")) != 0)
    log_warnx("unable to register to sysORTable (%d)", rc);
}
