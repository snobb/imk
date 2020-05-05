/*
 *  author: Aleksei Kozadaev (2019)
 */

#include "build_host.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "log.h"
#include "cfg.h"
#include "cmd.h"
#include "files.h"

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

    if (argc == 1) {
        usage(argv[0]);
    }

    cfg->sleep_del = 300;

    while ((ch = getopt(argc, argv, "hvc:t:S:orwk:")) != -1) {
        switch (ch) {
            case 'h':
                usage(argv[0]);
                exit(EXIT_SUCCESS);

            case 'v':
                printf("%s\n", VERSION);
                exit(EXIT_SUCCESS);

            case 'c':
                cfg->cmd->path = optarg;
                break;

            case 'o':
                cfg->onerun = 1;
                break;

            case 't':
                cfg->threshold = atoi(optarg);
                break;

            case 'S':
                cfg->sleep_del = atoi(optarg);
                break;

            case 'r':
                cfg->recurse = 1;
                break;

            case 'w':
                cfg->cmd->spawn = true;
                break;

            case 'k':
                cfg->cmd->timeout_ms = atoi(optarg);
                break;

            default:
                LOG_ERR("error: unknown argument");
                exit(EXIT_FAILURE);
        }
    }

    if (cfg->recurse) {
        cfg->files = files_parse((const char**)argv + optind, &cfg->nfiles);
    } else {
        cfg->files = argv + optind;
        cfg->nfiles = argc - optind;
    }

    if (cfg->nfiles == 0) {
        LOG_ERR("error: no files to monitor");
        exit(EXIT_FAILURE);
    }

    if (cfg->cmd->path == NULL) {
        LOG_ERR("error: command was not specified");
        exit(EXIT_FAILURE);
    }

    if ((cfg->fds = malloc(sizeof(*cfg->fds) * cfg->nfiles)) == NULL) {
        LOG_PERROR("malloc");
        exit(EXIT_FAILURE);
    }
}

void
cfg_free(const struct config *cfg)
{
    if (cfg->fds) {
        free(cfg->fds);
    }

    files_free();
}

void
usage(const char *pname)
{
    fprintf(stdout,
            "usage: %s [-h] [-v] -c <command> [-t <sec>] [-S <ms>] [-o] [-r] [-w] [-k <ms>] "
            "<file ...> <dir>\n\n   The options are as follows:\n"
            "      -h         - display this text and exit\n"
            "      -v         - display the version\n"
            "      -c <cmd>   - command to execute when event is triggered\n"
            "      -t <sec>   - number of seconds to skip after the last executed "
            "command (default 0)\n"
            "      -S <ms>    - number of ms to sleep before reattaching in case of "
            "DELETE event (default 300)\n"
            "      -o         - exit after first iteration\n"
            "      -r         - if a directory is supplied, add all its sub-directories "
            "as well\n"
            "      -w         - spawn a subprocess for command\n"
            "      -k <ms>    - timeout after which to kill the command subproces "
            "(default - do not kill, assumes -w)\n"
            "      <file ...> - list of files to monitor\n\n"
            "   Please use quotes around the command if it is composed of multiple "
            "words\n\n", pname);
}
