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
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <stdint.h>
#include "interp.h"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "version.h"

static int msoak_log(lua_State *lua);
static int msoak_strftime(lua_State *lua);

/*
 * Invoke the function `name' in the Lua script, which _may_ return
 * an integer that we, in turn, return to caller.
 */

static int l_function(lua_State *L, char *name)
{
	int rc = 0;

	lua_getglobal(L, name);
	if (lua_type(L, -1) != LUA_TFUNCTION) {
		// fprintf(stderr, "Cannot invoke Lua function %s: %s\n", name, lua_tostring(L, -1));
		rc = 1;
	} else {
		lua_call(L, 0, 1);
		rc = (int)lua_tonumber(L, -1);
		lua_pop(L, 1);
	}
	lua_settop(L, 0);
	return (rc);
}

struct luadata *interp_init(char *script, bool verbose)
{
	struct luadata *luad;
	int rc;

	if ((luad = malloc(sizeof(struct luadata))) == NULL)
		return (NULL);

	luad->script = strdup(script);
	// luad->L = lua_open();
	luad->L = luaL_newstate();
	luaL_openlibs(luad->L);

	/*
	 * Set up a global with some values.
	 */

	lua_newtable(luad->L);
		lua_pushstring(luad->L, VERSION);
		lua_setfield(luad->L, -2, "version");

		lua_pushboolean(luad->L, verbose);
		lua_setfield(luad->L, -2, "verbose");

		// lua_pushstring(luad->L, "/Users/jpm/Auto/projects/on-github/owntracks/recorder/lua");
		// lua_setfield(luad->L, -2, "luapath");

		lua_pushcfunction(luad->L, msoak_log);
		lua_setfield(luad->L, -2, "log");

		lua_pushcfunction(luad->L, msoak_strftime);
		lua_setfield(luad->L, -2, "strftime");

	lua_setglobal(luad->L, "msoak");

	// fprintf(stderr, "initializing Lua hooks from `%s'\n", script);

	/* Load the Lua script */
	if (luaL_dofile(luad->L, luad->script)) {
		fprintf(stderr, "Cannot load Lua from %s: %s\n", luad->script, lua_tostring(luad->L, -1));
		return (NULL);
	}

	rc = l_function(luad->L, "init");
#if 0
	if (rc != 0) {

		/*
		 * After all this work (sigh), the Script has decided we shouldn't use
		 * hooks, so unload the whole Lua stuff and NULL back.
		 */

		interp_exit(luad, "init() returned non-zero");
		luad = NULL;
	}
#endif

	return (luad);
}


void interp_exit(struct luadata *luad, char *reason)
{
	if (luad && luad->script) {
		l_function(luad->L, "exit");
		// fprintf(stderr, "unloading Lua: %s\n", reason);
		free(luad->script);
		lua_close(luad->L);
		free(luad);
	}
}

char *interp_print(struct luadata *luad, char *fmtfunc, char *topic, JsonNode *json)
{
	char *_type = "unknown", *res;
	JsonNode *j;

	lua_settop(luad->L, 0);
	lua_getglobal(luad->L, fmtfunc);
	if (lua_type(luad->L, -1) != LUA_TFUNCTION) {
		fprintf(stderr, "cannot invoke %s in Lua script\n", fmtfunc);
		return (NULL);
	}

	lua_pushstring(luad->L, topic);			/* arg1: topic */

	if ((j = json_find_member(json, "_type")) != NULL) {
		if (j->tag == JSON_STRING)
			_type = j->string_;
	}

	lua_pushstring(luad->L, _type);			/* arg2: record type */

	lua_newtable(luad->L);				/* arg3: table */
	json_foreach(j, json) {
		if (j->tag >= JSON_ARRAY)
			continue;
		lua_pushstring(luad->L, j->key);		/* table key */
		if (j->tag == JSON_STRING) {
			lua_pushstring(luad->L, j->string_);
		} else if (j->tag == JSON_NUMBER) {
			lua_pushnumber(luad->L, j->number_);
		} else if (j->tag == JSON_NULL) {
			lua_pushnil(luad->L);
		} else if (j->tag == JSON_BOOL) {
			lua_pushboolean(luad->L, j->bool_);
		}
		lua_rawset(luad->L, -3);

	}

	/* Invoke format function in Lua with our args */
	if (lua_pcall(luad->L, 3, 1, 0)) {
		fprintf(stderr,  "Failed to run script: %s\n", lua_tostring(luad->L, -1));
		exit(1);
	}

	res = (char *)lua_tostring(luad->L, -1);
	return (res);
}

/*
 * These utility functions are provided within the Lua script.
 */

static int msoak_log(lua_State *lua)
{
	const char *str;

	if (lua_gettop(lua) >= 1) {
		str =  lua_tostring(lua, 1);
		fprintf(stderr, "MSOAKLOG: %s\n", str);
		lua_pop(lua, 1);
	}
	return 0;
}

/*
 * msoak.strftime(format, seconds)
 * Perform a strtime(3) for Lua with the specified format and
 * seconds, and return the string result to Lua. As a special
 * case, if `seconds' is negative, use current time.
 */

static int msoak_strftime(lua_State *lua)
{
	const char *fmt;
	long secs;
	struct tm *tm;
	char buf[BUFSIZ];

	if (lua_gettop(lua) >= 1) {
		fmt =  lua_tostring(lua, 1);
		if ((secs =  lua_tonumber(lua, 2)) < 1)
			secs = time(0);

		if ((tm = gmtime(&secs)) != NULL) {
			strftime(buf, sizeof(buf), fmt, tm);

			lua_pushstring(lua, buf);
			return (1);
		}
	}
	return (0);
}
