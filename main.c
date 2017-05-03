/*
 *  main.c
 *  author: Aleksei Kozadaev (2016)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <pwd.h>

#include "imk.h"
#include "compat.h"
#include "genarray.h"
#include "lua_interp.h"
#include "log.h"

void parse_args(int argc, char **argv);
int get_rc_file(char *rc_file_out);
void usage(const char *pname);

struct config cfg = { 0 };

const char * const RC_FILE = ".imkrc.lua";

int
main(int argc, char **argv)
{

    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    parse_args(argc, argv);

    cfg.lua = lua_init();
    if (cfg.lua == NULL) {
        LOG_ERR("cannot initialise LUA interpreter");
        exit(EXIT_FAILURE);
    }

    if (get_rc_file(cfg.rc_file) != -1) {
        lua_execute(cfg.lua, cfg.rc_file);
    }

    printf(":: [%s] start monitoring: cmd[%s] files[", get_time(),
            (cfg.cmd != NULL ? cfg.cmd : "none"));
    const char **files = cfg.files;
    while (*files != NULL) {
        fd_register(&cfg, *files);
        printf("%s", *files++);
        if (*files != NULL) {
            printf(" ");
        }
    }
    printf("]\n");

    fd_dispatch(&cfg);
    fd_close(&cfg);
    lua_close(cfg.lua);

    return EXIT_SUCCESS;
}

void
parse_args(int argc, char **argv)
{
    int ch;
    opterr = 0;

    memset(&cfg, 0, sizeof(cfg));

    if (argc == 1) {
        usage(argv[0]);
    }

    while ((ch = getopt(argc, argv, "hc:")) != -1) {
        switch (ch) {
            case 'h':
                usage(argv[0]);
                exit(EXIT_SUCCESS);

            case 'c':
                cfg.cmd = optarg;
                break;

            default:
                LOG_ERR("unknown argument");
                exit(EXIT_FAILURE);
        }
    }

    cfg.files = (const char**)argv + optind;

    if (*cfg.files == NULL) {
        LOG_ERR("no files to monitor");
        exit(EXIT_FAILURE);
    }
}

int
get_rc_file(char *rc_file_out)
{
    char *homedir;

    snprintf(rc_file_out, FILENAME_MAX, "%s", RC_FILE);
    if (access(rc_file_out, F_OK) != -1) {
        LOG_DEBUG("found lua config %s", rc_file_out);
        return 0;
    }

    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }

    snprintf(rc_file_out, FILENAME_MAX, "%s/%s", homedir, RC_FILE);
    if (access(rc_file_out, F_OK) != -1) {
        LOG_DEBUG("found lua config %s", rc_file_out);
        return 0;
    }

    return -1;
}

void
usage(const char *pname)
{
    fprintf(stdout,
            "usage: %s [-h] -c <command> <file ...>\n\n"
            "   The options are as follows:\n"
            "      -h          - display this text and exit\n"
            "      -c <cmd>    - command to execute when event is triggered\n"
            "      <file ...>  - list of files to monitor\n\n"
            "   Please use quotes around the command if it is composed of "
            "multiple words\n\n"
            "   Lua event handlers are searched in .luarc.lua file in the "
            "current directory and then in $HOME\n\n", pname);
}

