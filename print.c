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

#include "conn.h"
#include "print.h"
#include "utstring.h"
#include "interp.h"

void printout(struct userdata *ud, char *topic, char *payload)
{
	JsonNode *json = NULL;
	conn *c = ud->c;
	char *res;

	if (!payload || !*payload) {
		return;
	}

        if (!c->fmt || !*c->fmt) {
                fprintf(stdout, "%s\n", payload);
		return;
	}

	/* If payload isn't JSON, return silently */
	if (*payload == '{') {
		if ((json = json_decode(payload)) != NULL) {
			/*
			 * add some bits and pieces from our connection so that the
			 * invokee can discover where this data originates from.
			 */

			json_append_member(json, "_conn_id", json_mkstring(c->id));
			json_append_member(json, "_conn_host", json_mkstring(c->host));
			json_append_member(json, "_conn_port", json_mknumber(c->port));
			json_append_member(json, "_conn_topic", json_mkstring(topic));


			if ((res = interp_print(ud->luad, c->fmt, topic, json)) != NULL) {
				printf("%s\n", res);
			}
			json_delete(json);
		}
	}
}

