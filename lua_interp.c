/*
 *  lua_interp.c
 *  author: Aleksei Kozadaev (2017)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "imk.h"
#include "log.h"
#include "lua_interp.h"

static int setup_cb(lua_State *L, const char *ev_table,
        const char *event_name);

static int call_cb(lua_State *L, const char *event_name,
        size_t nargs);

static int trigger_event(lua_State *L, const char *event,
        const char **args, size_t nargs);

static void lua_print_error(lua_State *L);

/* api calls */
int api_shell(lua_State *L);
int api_command(lua_State *L);
int api_get_files(lua_State *L);


/* globals */
extern struct config cfg;

static const char * const EVENTS_TABLE = "events";
static const char * const EVENT_OPEN = "open";
static const char * const EVENT_CLOSE = "close";
static const char * const EVENT_CREATE = "create";
static const char * const EVENT_READ = "read";
static const char * const EVENT_WRITE = "write";
static const char * const EVENT_DELETE = "delete";


lua_State*
lua_init(void)
{
    lua_State *L = luaL_newstate();

    /* Make standard libraries available in the Lua object */
    luaL_openlibs(L);
    luaopen_table(L);

    /* register api functions */
    lua_register(L, "imk_shell", api_shell);
    lua_register(L, "imk_command", api_command);
    lua_register(L, "imk_getfiles", api_get_files);

    /* global events registry */
    lua_newtable(L);
    lua_setglobal(L, EVENTS_TABLE);

    return L;
}

void
lua_execute(lua_State *L, const char* fname)
{
    int result;

    LOG_DEBUG("loading lua config %s", fname);

    /* Load the program; this supports both source code and bytecode files. */
    result = luaL_loadfile(L, fname);
    if (result != LUA_OK) {
        lua_print_error(L);
        return;
    }

    /*
     * Finally, execute the program by calling into it.
     * Change the arguments if you're not running vanilla Lua code.
     */
    result = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (result != LUA_OK) {
        lua_print_error(L);
        return;
    }
}

void
lua_print_error(lua_State *L)
{
    LOG_ERR_VA("%s", lua_tostring(L, -1));
    lua_pop(L, 1);
}

int
lua_trigger_open(lua_State *L, const char *fname)
{
    LOG_DEBUG("open event - %s", fname);
    return trigger_event(L, EVENT_OPEN, &fname, 1);
}

int
lua_trigger_close(lua_State *L, const char *fname)
{
    LOG_DEBUG("close event - %s", fname);
    return trigger_event(L, EVENT_CLOSE, &fname, 1);
}

int
lua_trigger_create(lua_State *L, const char *fname)
{
    LOG_DEBUG("create event - %s", fname);
    return trigger_event(L, EVENT_CREATE, &fname, 1);
}

int
lua_trigger_read(lua_State *L, const char *fname)
{
    LOG_DEBUG("read event - %s", fname);
    return trigger_event(L, EVENT_READ, &fname, 1);
}

int
lua_trigger_write(lua_State *L, const char *fname)
{
    LOG_DEBUG("write event - %s", fname);
    return trigger_event(L, EVENT_WRITE, &fname, 1);
}

int
lua_trigger_delete(lua_State *L, const char *fname)
{
    LOG_DEBUG("delete event - %s", fname);
    return trigger_event(L, EVENT_DELETE, &fname, 1);
}

static int
trigger_event(lua_State *L, const char *event,
        const char **args, size_t nargs)
{
    int result = LUA_OK;

    if ((result = setup_cb(L, EVENTS_TABLE, event)) == LUA_OK) {
        /* pass agruments */
        for (int i = 0; i < nargs; ++i) {
            lua_pushstring(L, args[i]);
        }

        result = call_cb(L, event, nargs);
        if (result != LUA_OK) {
            lua_print_error(L);
            lua_pop(L, nargs);
        }
        else {
            lua_pop(L, 1);
        }
    }

    return result;
}

static int
setup_cb(lua_State *L, const char *ev_table, const char *key)
{
    LOG_DEBUG("setup_cb stack size %d", lua_gettop(L));

    // calling a function defined in .lua
    lua_getglobal(L, EVENTS_TABLE);
    if (!lua_istable(L, -1)) {
        LOG_ERR_VA("%s is not a table", EVENTS_TABLE);
        exit(1);
    }

    lua_pushstring(L, key);
    lua_gettable(L, -2);

    if (lua_isnil(L, -1)) {
        lua_pop(L, 2);
        return LUA_ERRRUN;
    }

    return LUA_OK;
}

static int
call_cb(lua_State *L, const char *key, size_t nargs)
{
    LOG_DEBUG("call_cb stack size %d", lua_gettop(L));

    if (!lua_isfunction(L, -1 - nargs)) {
        LOG_ERR_VA("%s.%s is not a function\n", EVENTS_TABLE, key);
        return LUA_ERRRUN;
    }

    return lua_pcall(L, nargs, 0, 0);
}

/* API functions */
int
api_shell(lua_State *L)
{
    int nargs = lua_gettop(L);

    if (nargs != 1) {
        luaL_error(L, "shell: 1 argument expected");
    }

    if (lua_type(L, 1) != LUA_TSTRING) {
        return luaL_error(L, "shell: the argument must be a string");
    }

    const char *cmd = lua_tostring(L, 1);

    return (system(cmd) == 0) ? LUA_OK : LUA_ERRRUN;
}

int
api_get_files(lua_State *L)
{
    int nargs = lua_gettop(L);

    if (nargs > 0) {
        return luaL_error(L, "get_files: too many arguments");
    }

    lua_newtable(L);
    for (int i = 0; cfg.files[i] != NULL; ++i) {
        lua_pushnumber(L, i + 1);
        lua_pushstring(L, cfg.files[i]);
        lua_settable(L, -3);
    }

    return 1;  /* returning 1 table */
}

int
api_command(lua_State *L)
{
    int nargs = lua_gettop(L);

    if (nargs > 0) {
        return luaL_error(L, "command: too many arguments");
    }

    if (cfg.cmd != NULL) {
        return (system(cfg.cmd) == 0) ? LUA_OK : LUA_ERRRUN;
    }

    return LUA_OK;
}
