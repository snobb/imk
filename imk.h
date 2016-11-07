#ifndef __IMK_H__
#define __IMK_H__

#include "gen_array.h"

struct config {
    const char *cmd;
    const char **files;
    ARRAY_STRUCT(fd, int) fds;
};

#endif /* __IMK_H__ */
