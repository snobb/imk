#ifndef __IMK_H__
#define __IMK_H__

struct config {
    int  threshold;
    int  onerun;
    unsigned sleep_del;
    const char *cmd;
    const char **files;
    size_t nfiles;
    /* the fds array MUST have the same order as files */
    int *fds;
    size_t fds_size;
};

int run_command(const char *cmd);

#endif /* __IMK_H__ */
