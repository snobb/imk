#ifndef __CFG_H__
#define __CFG_H__

#include <stdbool.h>
#include <stdlib.h>

struct config {
    int threshold;
    int sleep_delay;

    struct command *cmd;
    char **files;
    size_t nfiles;

    /* the fds array MUST have the same order as files */
    int *fds;
    size_t nfds;

    bool onerun;
    bool recurse;
    bool immediate_run;
};


void cfg_add_fd(struct config *cfg, int fd);
void cfg_parse_args(struct config *cfg, int argc, char **argv);
void cfg_free(const struct config *cfg);
void cfg_print_header(const struct config *cfg);
void cfg_register_files(const struct config *cfg);

#endif /* __CFG_H__ */
