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

#include <stdio.h>
#include <stdlib.h>
#include "conn.h"

#define MAXCONF		10	/* maximum servers in config */

conn **load_conn(char *filename)
{
	conn **conn_list, *c;
	config_t config, *cf;
	config_setting_t *servers;

	config_init(cf = &config);

	if (config_read_file(&config, filename) != CONFIG_TRUE) {
		fprintf(stderr, "Error on line %d of %s: %s\n",
			config_error_line(cf),
			config_error_file(cf),
			config_error_text(cf));
		return (NULL);
	}

	if ((conn_list = malloc(sizeof(conn) * (MAXCONF + 1))) == NULL)
		return (NULL);

	/*
	 * In the config we have "servers" which is an array
	 *
	 * servers = ( { id = "H"
	 * 	      host = "localhost"
	 * 	      port = 1888
	 * 	      topics = [
	 * 	     	"owntracks/jpm/+",
	 * 		"test",
	 * 		"nothing" ]
	 * 	    } )
	 */

	servers = config_lookup(cf, "servers");
	if (servers != NULL) {
		int n, nservers = config_setting_length(servers);

		for (n = 0; n < nservers; n++) {
			const char *host, *user = NULL, *pass = NULL, *v;
			char id[12];
			int iv;

			c = (conn *)calloc(sizeof(conn), sizeof(char));
			conn_list[n] = c;

			config_setting_t *serv = config_setting_get_elem(servers, n);

			c->id = NULL;
			if (config_setting_lookup_string(serv, "id", &v)) {
				c->id = strdup(v);
			}
			if (c->id == NULL) {
				snprintf(id, sizeof(id), "%02d", n);
				c->id = strdup(id);
			}

			c->host = NULL;
			if (config_setting_lookup_string(serv, "host", &host)) {
				c->host = strdup(host);
			}
			if (c->host == NULL) c->host = strdup("localhost");

			c->user = NULL;
			if (config_setting_lookup_string(serv, "user", &user)) {
				c->user = strdup(user);
			}

			c->cacert = NULL;
			if (config_setting_lookup_string(serv, "cacert", &v)) {
				c->cacert = strdup(v);
			}

			c->pass = NULL;
			if (config_setting_lookup_string(serv, "passenv", &pass)) {
				if (pass && *pass) {
					pass = getenv(pass + 1);
					if (pass && *pass) {
						c->pass = strdup(pass);
					}
				}
			} else {
				if (config_setting_lookup_string(serv, "pass", &pass)) {
					if (pass && *pass)
						c->pass = strdup(pass);
				}
			}

			config_setting_t *topicarray = config_setting_get_member(serv, "topics");
			if (topicarray != NULL) {
				int i;
				const char *t;

				utarray_new(c->topics, &ut_str_icd);


				for (i = 0; i < config_setting_length(topicarray); i++) {
					if ((t = config_setting_get_string_elem(topicarray, i)) != NULL) {
						// printf("%d %s\n", i, t);
						utarray_push_back(c->topics, &t);
					}
				}
			}

			c->showid = true;
			if (config_setting_lookup_bool(serv, "showid", &iv) == CONFIG_TRUE) {
				c->showid = iv;
			}

			c->showtopic = true;
			if (config_setting_lookup_bool(serv, "showtopic", &iv) == CONFIG_TRUE) {
				c->showtopic = iv;
			}

			c->port = 1883;
			if (config_setting_lookup_int(serv, "port", &iv) == CONFIG_TRUE) {
				c->port = iv;
			}
		}
		conn_list[n] = NULL;
	}

	config_destroy(&config);
	return (conn_list);
}

void free_conn(conn **config)
{
	conn **cp;

	for (cp = config; cp && *cp; cp++) {
		conn *c = *cp;
		if (c->id)	free(c->id);
		if (c->host)	free(c->host);
		if (c->user)	free(c->user);
		if (c->pass)	free(c->pass);
		if (c->cacert)	free(c->cacert);

		if (c->topics) {
			utarray_free(c->topics);
		}
		free(c);
	}
	free(config);
}

#ifdef TESTING
int main()
{
	conn **conn, *c;
	int n = 0;

	conn = load_conn("test.config");
	if (conn == NULL) {
		fprintf(stderr, "Can't load conf\n");
		return 1;
	}

	for (n = 0; conn[n] != NULL; n++) {
		char **t;
		c = conn[n];
		printf("%3d --> id=[%s]\n", n, c->id);

		t = NULL;
		while ((t = (char **)utarray_next(c->topics, t))) {
			printf("            topic=[%s]\n", *t);
		}
	}
	free_conn(conn);
	return 0;
}
#endif
