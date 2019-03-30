/*
 * Copyright (C) 2019 Jan-Piet Mens <jp@mens.de>
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

#ifndef _CONF_H_
#define _CONF_H_

#include <stdbool.h>
#include <libconfig.h>
#include <mosquitto.h>
#include "utarray.h"

struct conn {
	char *id;			/* mandatory: connection identifier */
	char *host;			/* MQTT broker */
	short port;
	char *user;
	char *pass;			/* password for user, or .. */
	char *passenv;			/* .. name of environment variable containing pass */
	char *cacert;
	bool showtopic;			/* whether or not to print out topic with values  */
	UT_array *topics;
	struct mosquitto *mosq;

};
typedef struct conn conn;

conn **load_conn(char *filename);
void free_conn(conn **conn_list);

#endif
