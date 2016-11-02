#ifndef __IMK_H__
#define __IMK_H__

#include "gen_array.h"

ARRAY_STRUCT(fd, int);

struct config {
    const char *cmd;
    const char **files;
    struct array_fd fds;
};

#endif /* __IMK_H__ */
