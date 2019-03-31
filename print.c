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

#if 0
static char *boolean(bool bf)
{
	// obtain externally
	//
	
	return (bf) ? "AN" : "AUS";
}

static void substitute(char *fmt, JsonNode *json)
{
	JsonNode *e;
	static UT_string *ut, *kw;
	char *bp;

	utstring_renew(ut);

	for (bp = fmt; bp && *bp; bp++) {
		if (*bp == '\\' && *(bp + 1)) {
			utstring_printf(ut, "%c", *++bp);
			continue;
		} else if (*bp == '{') {
			char *f = NULL;
			utstring_renew(kw);
			for (++bp; bp && *bp && *bp != '}'; ) {	/* skip over { */
				utstring_printf(kw, "%c", *bp++);
			}

			/* kw might contain a printf-type format string, for example
			 * "lat:8.4f". Truncate at colon, and save the rest as fmt
			 */

			f = strchr(utstring_body(kw), ':');
			if (f != NULL) {
				*f++ = 0;	/* f now has "%6d" or "%-10s" in it */
				// printf("PRINTF=[%s]\n", f);
			}
			// printf("KW=[%s]\n", utstring_body(kw));

			if ((e = json_find_member(json, utstring_body(kw))) != NULL) {
				//static char *jtypes[] = { "NULL", "BOOL", "STRING", "NUMBER",
				//			  "ARRAY", "OBJECT" };

				// printf("json type == %s\n", jtypes[e->tag]);
				if (e->tag == JSON_STRING) {
					f = (f) ? f : "%s";
					utstring_printf(ut, f, e->string_);
				} else if (e->tag == JSON_NUMBER) {
					f = (f) ? f : "%2lg";
					utstring_printf(ut, f, e->number_);
				} else if (e->tag == JSON_BOOL) {
					f = (f) ? f : "%s";
					utstring_printf(ut, f, boolean(e->bool_));
				}
			}

			// printf("REST=--->%s<---\n", bp);

			continue;
		}
		utstring_printf(ut, "%c", *bp);
	}

	printf("%s\n", utstring_body(ut));
}
#endif

void printout(struct userdata *ud, char *topic, char *payload)
{
	JsonNode *json;
	conn *c = ud->c;
	char *res;

	if (!payload || !*payload) {
		return;
	}

        if (!c->fmt) {
                fprintf(stdout, "%s\n", payload);
		return;
	}

	/* If payload isn't JSON, return silently */

	if (*payload != '{' || ((json = json_decode(payload)) == NULL)) {
		return;
	}

	res = interp_print(ud->luad, c->fmt, topic, json);
	if (res != NULL) {
		printf("%s\n", res);
	}

	// substitute(c->fmt, json);
	json_delete(json);
}

