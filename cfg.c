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

#define DEFAULT_SLEEP_DELAY 300

static void usage(const char *pname);

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

    cfg->sleep_delay = DEFAULT_SLEEP_DELAY;

    while ((ch = getopt(argc, argv, "c:d:hk:orsS:t:vw")) != -1) {
        switch (ch) {
            case 'h':
                usage(argv[0]);
                exit(EXIT_SUCCESS);

            case 'c':
                cfg->cmd->path = optarg;
                break;

            case 'd':
                cfg->cmd->teardown = optarg;
                cfg->cmd->spawn = true;
                break;

            case 'k':
                cfg->cmd->timeout_ms = atoi(optarg);
                cfg->cmd->spawn = true;
                break;

            case 'o':
                cfg->onerun = true;
                break;

            case 'r':
                cfg->recurse = true;
                break;

            case 's':
                cfg->cmd->wrap_shell = false;
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
                cfg->cmd->spawn = true;
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
cfg_print_header(const struct config *cfg)
{
    printf(":: [%s] start monitoring: ", get_time());
    cmd_print_header(cfg->cmd);

    if (cfg->threshold > 0) {
        printf("threshold[%d] ", cfg->threshold);
    }

    if (cfg->sleep_delay != DEFAULT_SLEEP_DELAY) {
        printf("sleep-delay[%d] ", cfg->sleep_delay);
    }

    if (cfg->onerun) {
        printf("once ");
    }

    if (cfg->recurse) {
        printf("recurse ");
    }

    printf("files[");

    for (int i = 0; i < cfg->nfiles; ++i) {
        if (i < cfg->nfiles - 1) {
            printf("%s ", cfg->files[i]);
        } else {
            printf("%s", cfg->files[i]);
        }
    }

    printf("]\n");
}

void
usage(const char *pname)
{
    fprintf(stdout, "usage: %s -c <command> [options] <file/dir ...>\n\n"
            "   The options are as follows:\n"
            "      -c <cmd>   - command to execute when event is triggered\n"
            "      -d <cmd>   - teardown command to execute when -k timeout occurs "
            "(assumes -w). The PID is available in CMD_PID environment variable.\n"
            "      -h         - display this text and exit\n"
            "      -k <ms>    - timeout after which to kill the command subproces "
            "(default - do not kill. Assumes -w.)\n"
            "      -o         - exit after the first iteration\n"
            "      -r         - if a directory is supplied, add all its sub-directories "
            "as well\n"
            "      -s         - do not run the command inside a shell (eg. /bin/sh -c <cmd>)\n"
            "      -S <ms>    - number of ms to sleep before reattaching in case of "
            "DELETE event (default 300)\n"
            "      -t <sec>   - number of seconds to skip after the last executed "
            "command (default 0)\n"
            "      -v         - display the version [%s]\n"
            "      -w         - spawn a subprocess for command.\n"
            "      <file/dir ...> - list of files or folders to monitor\n\n"
            "   Please use quotes around the command and teardown command if it is "
            "composed of multiple words\n\n", pname, VERSION);
}
