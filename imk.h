#ifndef __IMK_H__
#define __IMK_H__

#include "gen_array.h"

struct config {
    int  threshold;
    int  onerun;
    unsigned sleepDelete;
    const char *cmd;
    const char **files;
    /* the fds array MUST have the same order as files */
    ARRAY_STRUCT(fd, int) fds;
};

int run_command(const char *cmd);

#endif /* __IMK_H__ */
