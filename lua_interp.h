#ifndef __LUA_INTERP_H__
#define __LUA_INTERP_H__

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

lua_State* lua_init(void);
void lua_execute(lua_State *state, const char* fname);
int lua_trigger_open(lua_State *L, const char *fname);
int lua_trigger_close(lua_State *L, const char *fname);
int lua_trigger_create(lua_State *L, const char *fname);
int lua_trigger_read(lua_State *L, const char *fname);
int lua_trigger_write(lua_State *L, const char *fname);
int lua_trigger_delete(lua_State *L, const char *fname);

#endif /* __LUA_INTERP_H__ */
