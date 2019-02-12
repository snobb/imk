#ifndef __CFG_H__
#define __CFG_H__

struct config {
    int threshold;
    int onerun;
    int sleep_del;

    const char *cmd;
    const char **files;
    size_t nfiles;

    /* the fds array MUST have the same order as files */
    int *fds;
    size_t nfds;
};


void cfg_add_fd(struct config *cfg, int fd);
void cfg_parse_args(struct config *cfg, int argc, char **argv);
void cfg_clean(const struct config *cfg);

#endif /* __CFG_H__ */