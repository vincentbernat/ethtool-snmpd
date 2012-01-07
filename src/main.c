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

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include <signal.h>
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
  if (!debug && (daemon(0, 0) != 0))
    fatal("failed to detach daemon");

  /* Main loop */
  keep_running = 1;
  signal(SIGTERM, stop_server);
  signal(SIGINT, stop_server);  
  while (keep_running)
    agent_check_and_process(1);

  return 0;
}
