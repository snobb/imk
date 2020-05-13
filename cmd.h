#ifndef __CMD_H__
#define __CMD_H__

#include <stdbool.h>

struct command {
    bool no_spawn;
    int timeout_ms;
    const char *path;
    const char *teardown;
};

struct command cmd_make();
int cmd_run(const struct command *cmd);
void cmd_print_header(const struct command *cmd);

#endif /* __CMD_H__ */
