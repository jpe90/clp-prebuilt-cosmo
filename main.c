/*-*- mode:c;indent-tabs-mode:t;c-basic-offset:2;tab-width:8;coding:utf-8   -*-│
/*-*- mode:c;indent-tabs-mode:t;c-basic-offset:2;tab-width:8;coding:utf-8   -*-│
│vi: set net ft=c ts=2 sts=2 sw=2 fenc=utf-8                                :vi│
╚─────────────────────────────────────────────────────────────────────────────*/
#include "lpcode.h"
#include "lpcap.h"
#include "lpprint.h"
#include "lptree.h"
#include "lptypes.h"

#include "lauxlib.h"
#include "lualib.h"
#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static
#include "optparse.h"

static const char *progname = "clp";
static const char *default_lua_path = "/zip/.lua/?.lua;/zip/.lua/?/init.lua";

static bool using_highlight_line;
static bool overrides_filetype;
static bool list_overrides;

static char *overriding_filetype;
static int highlight_line;
static char *filename;

int luaopen_lpeg(lua_State *L);
bool lua_path_add(lua_State *L, const char *path);

bool lua_path_add(lua_State *L, const char *path) {
	if (!L || !path)
		return false;
	lua_getglobal(L, "package");
	lua_pushstring(L, path);
	lua_pushstring(L, "/?.lua;");
	lua_getfield(L, -3, "path");
	lua_concat(L, 3);
	lua_setfield(L, -2, "path");
	lua_pop(L, 1);
	return true;
}

static long ParseInt(const char *s) {
	return strtol(s, 0, 0);
}

static void usage () {
  lua_writestringerror("%s: ", progname);
  lua_writestringerror(
  "usage: %s [options] [file]\n"
  "Available options are:\n"
  "  -l       list types of filetypes that can be forced       \n"
  "  -t       force a language's syntax for highlighting a file\n"
  "  -h       Highlight a non-blank line		       \n"
  ,
  progname);
}

static void ProgramOverrideFiletype(char *ft) {
	overrides_filetype = true;
	overriding_filetype = ft;
}

static void ProgramHighlightLine(int line) {
	using_highlight_line = true;
	highlight_line = line;
}

static void LuaStart(lua_State *L) {
	luaL_openlibs(L);
	luaL_getsubtable(L, LUA_REGISTRYINDEX, "_PRELOAD");
	lua_pushcfunction(L, luaopen_lpeg);
	lua_setfield(L, -2, "lpeg");
	lua_pop(L, 1);
}

static void GetOpts(char *argv[]) {
	int option;
	struct optparse_long longopts[] = {
		{"highlight-line", 'h', OPTPARSE_REQUIRED},
		{"override-filetype", 't', OPTPARSE_REQUIRED},
		{"list-overrides", 'l', OPTPARSE_NONE},
		{0}};
	struct optparse options;
	optparse_init(&options, argv);
	while ((option = optparse_long(&options, longopts, NULL)) != -1) {
		switch (option) {
		case 'l':
			list_overrides = true;
			break;
		case 't':
			ProgramOverrideFiletype(options.optarg);
			break;
		case 'h':
			ProgramHighlightLine(ParseInt(options.optarg));
			break;
		}
	}

	filename = optparse_arg(&options);
}

static int pmain (lua_State *L) {
	char **argv = (char **)lua_touserdata(L, 2);
	LuaStart(L);
	if(!lua_path_add(L,default_lua_path)) {
		fprintf(stderr, "Failed to set path\n");
	}
	int status = 0;
	GetOpts(argv);
	if (!filename) {
    	usage();
		printf("Usage: clp [options] file\n");
		return 1;
	}
	lua_getglobal(L, "require");
	lua_pushstring(L, "clp");
	status = lua_pcall(L, 1, 1, 0);
	if (status != 0)
		fprintf(stderr, "%s\n", lua_tostring(L, -1));

	lua_getglobal(L, "main");
	lua_newtable(L);
	lua_pushliteral(L, "filename");
	lua_pushstring(L, filename);
	lua_settable(L, -3);
	if(using_highlight_line) {
		lua_pushliteral(L, "highlight_line");
		lua_pushinteger(L, highlight_line);
		lua_settable(L, -3);
	}
	if(overrides_filetype) {
		lua_pushliteral(L, "overriding_filetype");
		lua_pushstring(L, overriding_filetype);
		lua_settable(L, -3);
	}
	status = lua_pcall(L, 1, 0, 0);
	if (status != 0) {
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
		return 1;
	}

	lua_pushboolean(L, 1);  /* signal no errors */
	return 1;
}


int main (int argc, char **argv) {
	lua_State *L;
	int status, result;
	L = luaL_newstate();  /* create state */
	if (L == NULL) {
		fprintf(stderr,"cannot create state: not enough memory\n");
		return EXIT_FAILURE;
	}
	lua_pushcfunction(L, &pmain);  /* to call 'pmain' in protected mode */
	lua_pushinteger(L, argc);  /* 1st argument */
	lua_pushlightuserdata(L, argv); /* 2nd argument */
	status = lua_pcall(L, 2, 1, 0);  /* do the call */
	result = lua_toboolean(L, -1);  /* get result */
	if (status != 0) {
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
		return 1;
	}
	lua_close(L);

	return (result && status == LUA_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
}
