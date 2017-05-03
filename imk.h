#ifndef __IMK_H__
#define __IMK_H__

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "genarray.h"

struct config {
    const char *cmd;
    const char **files;
    char rc_file[FILENAME_MAX];
    ARRAY_STRUCT(fd, int) fds;
    lua_State *lua;
};

#endif /* __IMK_H__ */
