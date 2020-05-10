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

    cfg->sleep_delay = 300;

    while ((ch = getopt(argc, argv, "c:d:hk:orS:t:vw")) != -1) {
        switch (ch) {
            case 'h':
                usage(argv[0]);
                exit(EXIT_SUCCESS);

            case 'c':
                cfg->cmd->path = optarg;
                break;

            case 'd':
                cfg->cmd->teardown = optarg;
                break;

            case 'k':
                cfg->cmd->timeout_ms = atoi(optarg);
                break;

            case 'o':
                cfg->onerun = 1;
                break;

            case 'r':
                cfg->recurse = 1;
                break;

            case 'S':
                cfg->sleep_delay = atoi(optarg);
                break;

            case 't':
                cfg->threshold = atoi(optarg);
                break;

            case 'v':
                printf("%s\n", VERSION);
                exit(EXIT_SUCCESS);

            case 'w':
                cfg->cmd->no_spawn = true;
                break;

            default:
                LOG_ERROR("unknown argument");
                exit(EXIT_FAILURE);
        }
    }

    if (cfg->recurse) {
        cfg->files = files_parse((const char **) argv + optind, &cfg->nfiles);
    } else {
        cfg->files = argv + optind;
        cfg->nfiles = argc - optind;
    }

    if (cfg->nfiles == 0) {
        LOG_ERROR("no files to monitor");
        exit(EXIT_FAILURE);
    }

    if (cfg->cmd->path == NULL) {
        LOG_ERROR("command was not specified");
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
            "usage: %s -c <command> -d <command> [-h] [-k <ms>] [-o] [-r] [-S <ms>] [-t <sec>] "
            "[-v] [-w] <file ...> <dir>\n\n   The options are as follows:\n"
            "      -c <cmd>   - command to execute when event is triggered\n"
            "      -d <cmd>   - teardown command to execute when -k timeout occurs\n"
            "      -h         - display this text and exit\n"
            "      -k <ms>    - timeout after which to kill the command subproces "
            "(default - do not kill)\n"
            "      -o         - exit after the first iteration\n"
            "      -r         - if a directory is supplied, add all its sub-directories "
            "as well\n"
            "      -S <ms>    - number of ms to sleep before reattaching in case of "
            "DELETE event (default 300)\n"
            "      -t <sec>   - number of seconds to skip after the last executed "
            "command (default 0)\n"
            "      -v         - display the version [%s]\n"
            "      -w         - do not spawn a subprocess for command (not compatible "
            "with -k and -d)\n"
            "      <file/dir ...> - list of files or folders to monitor\n\n"
            "   Please use quotes around the command and teardown command if it is "
            "composed of multiple words\n\n", pname, VERSION);
}
