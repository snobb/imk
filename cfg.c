/*
 *  cfg.c
 *  author: Aleksei Kozadaev (2019)
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "log.h"
#include "cfg.h"

void usage(const char *pname);

inline void
cfg_add_fd(struct config *cfg, int fd)
{
    cfg->fds[cfg->nfds++] = fd;
}

void
cfg_parse_args(struct config *cfg, int argc, char **argv)
{
    int ch;
    opterr = 0;

    memset(cfg, 0, sizeof(*cfg));

    if (argc == 1) {
        usage(argv[0]);
    }

    cfg->sleep_del = 300;

    while ((ch = getopt(argc, argv, "hc:t:s:o")) != -1) {
        switch (ch) {
            case 'h':
                usage(argv[0]);
                exit(EXIT_SUCCESS);

            case 'c':
                cfg->cmd = optarg;
                break;

            case 'o':
                cfg->onerun = 1;
                break;

            case 't':
                cfg->threshold = atoi(optarg);
                break;

            case 's':
                cfg->sleep_del = atoi(optarg);
                break;

            default:
                LOG_ERR("error: unknown argument");
                exit(EXIT_FAILURE);
        }
    }

    cfg->files = (const char**)argv + optind;
    cfg->nfiles = argc - optind;

    if (cfg->cmd == NULL) {
        LOG_ERR("error: command was not specified");
        exit(EXIT_FAILURE);
    }

    if (*cfg->files == NULL) {
        LOG_ERR("error: no files to monitor");
        exit(EXIT_FAILURE);
    }

    cfg->fds = malloc(sizeof(*cfg->fds) * cfg->nfiles);
}

void
cfg_clean(const struct config *cfg)
{
    if (cfg->fds) {
        free(cfg->fds);
    }
}

void
usage(const char *pname)
{
    fprintf(stdout,
            "usage: %s [-h] -c <command> [-t <sec>] [-o] <file ...>\n\n"
            "   The options are as follows:\n"
            "      -h          - display this text and exit\n"
            "      -c <cmd>    - command to execute when event is triggered\n"
            "      -o          - exit after first iteration\n"
            "      -t <sec>    - number of seconds to skip after the last "
            "executed command (default 0)\n"
            "      -s <sec>    - number of milliseconds to sleep before "
            "reattaching in case of DELETE event (default 300)\n"
            "      <file ...>  - list of files to monitor\n\n"
            "   Please use quotes around the command if it is composed of "
            "multiple words\n\n", pname);
}
