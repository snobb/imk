#ifndef __CMD_H__
#define __CMD_H__

#include <stdbool.h>

struct command {
    const char *path;
    bool spawn;
    int timeout_ms;
    char **environ;
};

int cmd_run(const struct command *cmd);

#endif /* __CMD_H__ */
