/*
 * Copyright (C) 2019-2020 Jan-Piet Mens <jp@mens.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#ifdef __linux__
#include <bsd/string.h>
#endif
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <mosquitto.h>
#define SSL_VERIFY_PEER (1)
#define SSL_VERIFY_NONE (0)
#include <syslog.h>
#include <libgen.h>
#include <assert.h>
#include "ud.h"
#include "utstring.h"
#include "print.h"

static int run = true;

static void catcher(int signo)
{
	run = false;
}

static char *mosquitto_reason(int rc)
{

	static char *reasons[] = {
		"MOSQ_ERR_SUCCESS",		/*  0 */
		"MOSQ_ERR_NOMEM",		/*  1 */
		"MOSQ_ERR_PROTOCOL",		/*  2 */
		"MOSQ_ERR_INVAL",		/*  3 */
		"MOSQ_ERR_NO_CONN",		/*  4 */
		"MOSQ_ERR_CONN_REFUSED",	/*  5 */
		"MOSQ_ERR_NOT_FOUND",		/*  6 */
		"MOSQ_ERR_CONN_LOST",		/*  7 */
		"MOSQ_ERR_TLS",			/*  8 */
		"MOSQ_ERR_PAYLOAD_SIZE",	/*  9 */
		"MOSQ_ERR_NOT_SUPPORTED",	/* 10 */
		"MOSQ_ERR_AUTH",		/* 11 */
		"MOSQ_ERR_ACL_DENIED",		/* 12 */
		"MOSQ_ERR_UNKNOWN",		/* 13 */
		"MOSQ_ERR_ERRNO",		/* 14 */
		"MOSQ_ERR_EAI",			/* 15 */
		"MOSQ_ERR_PROXY",		/* 16 */
		"MOSQ_ERR_PLUGIN_DEFER",	/* 17 */
		"MOSQ_ERR_MALFORMED_UTF8",	/* 18 */
		"MOSQ_ERR_KEEPALIVE",		/* 19 */
		"MOSQ_ERR_LOOKUP"		/* 20 */
	};

	return ((rc >= 0 && rc <= MOSQ_ERR_LOOKUP) ? reasons[rc] : "unknown reason");
}

void on_connect(struct mosquitto *mosq, void *userdata, int reason)
{
	struct userdata *ud = (struct userdata *)userdata;
	conn *c = ud->c;
	char **topic = NULL;

	while ((topic = (char **)utarray_next(c->topics, topic))) {
		if (ud->verbose)
			printf("%s: subscribe to %s (q=%d) %s:%d\n",
				c->id, *topic, c->qos, c->host, c->port);
		mosquitto_subscribe(mosq, NULL, *topic, c->qos);
	}
}

void on_disconnect(struct mosquitto *mosq, void *userdata, int reason)
{
	struct userdata *ud = (struct userdata *)userdata;

	if (reason) {
		if (reason == MOSQ_ERR_ERRNO) {
			syslog(LOG_INFO, "Disconnected from %s. Reason: 0x%02X [%s] %m",
				 ud->c->id, reason, mosquitto_reason(reason));
		} else {
			syslog(LOG_INFO, "Disconnected from %s. Reason: 0x%02X [%s]",
				 ud->c->id, reason, mosquitto_reason(reason));
		}
	}
}

void on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *m)
{
	struct userdata *ud = (struct userdata *)userdata;
	conn *c = ud->c;

	/*
	 * mosquitto_message->
	 * 	 int mid;
	 * 	 char *topic;
	 * 	 void *payload;
	 * 	 int payloadlen;
	 * 	 int qos;
	 * 	 bool retain;
	 */

	if (m->retain && !c->showretained)
		return;

	if (c->showid) {
		fprintf(stdout, "%s ", c->id);
	}

	if (c->showtopic) {
		fprintf(stdout, "%s ", m->topic);
	}

	printout(ud, m->topic, (char *)m->payload);
	fflush(stdout);		/* TODO ?? */
}

int main(int argc, char **argv)
{
	char *progname, err[1024], clientid[256], *configfilename, *p;
	int rc;
	conn **conn_list, **cp, *c;
	struct userdata *ud;
	struct luadata *luad = NULL;
	gl *g;

	progname = basename(*argv);
	openlog(progname, LOG_PERROR, LOG_DAEMON);

	if (argc != 2) {
		fprintf(stderr, "Usage: %s file.config\n", *argv);
		return (2);
	}


	if ((conn_list = load_conn(argv[1], &g)) == NULL) {
		syslog(LOG_ERR, "Cannot open config at %s: %m", argv[1]);
		return (2);
	}
	configfilename = strdup(basename(argv[1]));
	if ((p = strrchr(configfilename, '.')) != NULL)
		*p = 0;

	if (g->luascript && *g->luascript) {
		luad = interp_init(g->luascript, g->verbose);
		if (luad == NULL) {
			syslog(LOG_ERR, "Stopping because loading of Lua script failed");
			exit(1);
		}
	}

	mosquitto_lib_init();
	signal(SIGINT, catcher);

	for (cp = conn_list; cp && *cp; cp++) {
		c = *cp;

		// show_conn(c);

		ud = (struct userdata *)calloc(1, sizeof (struct userdata));
		ud->luad = luad;
		ud->c = c;
		ud->verbose = g->verbose;

		if (c->clientid) {
			snprintf(clientid, sizeof(clientid), "%s", c->clientid);
		} else {
			/* use msoak-hostname:configfilename so that client can run on multiple hosts with same config  */
			char myhostname[128];

			if (gethostname(myhostname, sizeof(myhostname)) != 0)
				strlcpy(myhostname, "unknown", sizeof(myhostname));

			snprintf(clientid, sizeof(clientid), "msoak-%s:%s", myhostname, configfilename);
		}
		c->mosq = mosquitto_new(clientid, true, ud);
		if (!c->mosq) {
			syslog(LOG_ERR, "Error: Out of memory");
			mosquitto_lib_cleanup();
			return (1);
		}

		mosquitto_reconnect_delay_set(c->mosq,
					2,        /* delay */
					20,       /* delay_max */
					0);       /* exponential backoff */

		if (ud->c->user || ud->c->pass) {
			mosquitto_username_pw_set(c->mosq, ud->c->user, ud->c->pass);
		}

		mosquitto_connect_callback_set(c->mosq, on_connect);
		mosquitto_disconnect_callback_set(c->mosq, on_disconnect);
		mosquitto_message_callback_set(c->mosq, on_message);

		if (c->cacert != NULL) {
			rc = mosquitto_tls_set(c->mosq,
				c->cacert,	/* cafile */
				NULL,		/* capath */
				NULL,		/* certfile */
				NULL,		/* keyfile */
				NULL);		/* pw_callback() */

			if (rc != MOSQ_ERR_SUCCESS) {
				fprintf(stderr, "Cannot set TLS CA at %s: %s (check path names)\n",
				c->cacert, mosquitto_strerror(rc));
				exit(3);
			}

			mosquitto_tls_opts_set(c->mosq,
				SSL_VERIFY_PEER,
				NULL,		/* tls_version: "tlsv1.2", "tlsv1" */
				NULL);		/* ciphers */
		}


		// rc = mosquitto_connect(c->mosq, ud->c->host, ud->c->port, 60);
		rc = mosquitto_connect_async(c->mosq, ud->c->host, ud->c->port, 60);
		if (rc) {
			if (rc == MOSQ_ERR_ERRNO) {
				strerror_r(errno, err, 1024);
				syslog(LOG_ERR, "connecting to %s:%d: %s", ud->c->host, ud->c->port, err);
			} else {
				syslog(LOG_ERR, "unable to connect to %s:%d: (%d)", ud->c->host, ud->c->port, rc);
			}
			/* do not disconnect and exit; we're connecting asynchronously
			mosquitto_lib_cleanup();
			return rc;
			*/
		}

		if ((rc = mosquitto_loop_start(c->mosq)) != MOSQ_ERR_SUCCESS) {
			syslog(LOG_ERR, "cannot loop_start for %s:%d: rc=0x%X", ud->c->host, ud->c->port, rc);
			return (rc);
		}
	}

	while (run) {
		sleep(10);
	}

	for (cp = conn_list; cp && *cp; cp++) {
		c = *cp;

		mosquitto_disconnect(c->mosq);
		mosquitto_destroy(c->mosq);
	}
	free_conn(conn_list, g);

	if (luad) {
		interp_exit(luad, "shutting down");
	}

	mosquitto_lib_cleanup();

	return (0);
}
