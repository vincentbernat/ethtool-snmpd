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

#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "ethtool-snmpd.h"

#define PROGNAME "ethtool-snmpd"

static void
usage() {
  printf("usage: " PROGNAME " [-d] [-x ADDRESS]\n"
	 "\n"
	 "\t-d: enable debug mode\n"
	 "\t-x ADDRESS: connect to master agent at ADDRESS\n");
  exit(0);
}

static int keep_running = 1;
static RETSIGTYPE
stop_server(int a) {
  (void)a;
  keep_running = 0;
}

int
main(int argc, char **argv) {
  int   ch;
  int   debug         = 0;
  char *agentx_socket = NULL;

  /* Parse arguments */
  while ((ch = getopt(argc, argv, "dx:")) != EOF)
    switch(ch) {
    case 'd':
      debug++;
      break;
    case 'x':
      agentx_socket = optarg;
      break;
    default:
      fprintf(stderr, "unknown option %c\n", ch);
      usage();
    }
  if (optind != argc)
    usage();

  log_init(debug, PROGNAME);

  /* Initialize Net-SNMP subagent */
  log_info("Initializing Net-SNMP subagent");
  netsnmp_enable_subagent();
  if (NULL != agentx_socket)
    netsnmp_ds_set_string(NETSNMP_DS_APPLICATION_ID,
			  NETSNMP_DS_AGENT_X_SOCKET, agentx_socket);
  snmp_disable_log();
  if (debug)
    snmp_enable_stderrlog();
  else
    snmp_enable_syslog_ident(PROGNAME, LOG_DAEMON);

  /* Setup agent */
  init_agent("ethtoolAgent");
  init_ethtoolStatTable();
  init_snmp("ethtoolAgent");


  /* Detach as a daemon */
  log_debug("going into background");
  if (!debug && (daemon(0, 0) != 0))
    fatal("failed to detach daemon");
  const char *pidfile = ETHTOOL_SNMPD_PID_FILE;
  int pid;
  char *spid;
  if ((pid = open(pidfile, O_TRUNC | O_CREAT | O_WRONLY, 0666)) == -1)
    fatal(
      "unable to open pid file " ETHTOOL_SNMPD_PID_FILE
      " (or the specified one)");
  if (asprintf(&spid, "%d\n", getpid()) == -1)
    fatal(
      "unable to create pid file " ETHTOOL_SNMPD_PID_FILE
      " (or the specified one)");
  if (write(pid, spid, strlen(spid)) == -1)
    fatal(
      "unable to write pid file " ETHTOOL_SNMPD_PID_FILE
      " (or the specified one)");
  free(spid);
  close(pid);

  /* Main loop */
  keep_running = 1;
  signal(SIGTERM, stop_server);
  signal(SIGINT, stop_server);  
  while (keep_running)
    agent_check_and_process(1);

  return 0;
}
