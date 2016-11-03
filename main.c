/*
 *  main.c
 *  author: Aleksei Kozadaev (2016)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

#include "imk.h"
#include "compat.h"
#include "gen_array.h"
#include "log.h"

ARRAY_FUNCS(fd, int)

void parse_args(struct config *cfg, int argc, char **argv);
void usage(const char *pname);

int
main(int argc, char **argv)
{
    struct config cfg = { 0 };

    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    parse_args(&cfg, argc, argv);
    array_fd_init(&cfg.fds);

    printf(":: [%s] start monitoring: cmd[%s] files[", get_time(), cfg.cmd);
    const char **files = cfg.files;
    while(*files != NULL)
    {
        printf("%s", *files++);
        if (*files != NULL) {
            printf(" ");
        }
    }
    printf("]\n");

    files = cfg.files;
    while(*files != NULL) {
        int wd = fd_register(&cfg, *files++);
        array_fd_append(&cfg.fds, wd);
    }

    fd_dispatch(&cfg);

    fd_close(&cfg);
    array_fd_free(&cfg.fds);

    return 0;
}

void
parse_args(struct config *cfg, int argc, char **argv)
{
    int ch;
    opterr = 0;

    memset(cfg, 0, sizeof(*cfg));

    if (argc == 1) {
        usage(argv[0]);
    }

    while ((ch = getopt(argc, argv, "hc:")) != -1) {
        switch (ch) {
            case 'h':
                usage(argv[0]);
                exit(0);

            case 'c':
                cfg->cmd = optarg;
                break;

            default:
                printf("%c %s\n", optopt, optarg);
        }
    }

    cfg->files = (const char**)argv + optind;

    if (cfg->cmd == NULL) {
        LOG_ERR("error: command was not specified");
        exit(1);
    }

    if (*cfg->files == NULL) {
        LOG_ERR("error: no files to monitor");
        exit(1);
    }
}

void
usage(const char *pname)
{
    fprintf(stderr,
            "usage: %s [-h] -c <command> <file ...>\n\n"
            "   The options are as follows:\n"
            "      -h          - display this text and exit\n"
            "      -c <cmd>    - command to execute when event is triggered\n"
            "      <file ...>  - list of files to monitor\n\n"
            "   Please use quotes around the command if it is composed of "
            "multiple words\n\n", pname);
}

