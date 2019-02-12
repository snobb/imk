/*
 *  main.c
 *  author: Aleksei Kozadaev (2016)
 */

#include <stdio.h>
#include <stdlib.h>

#include "cfg.h"
#include "compat.h"
#include "log.h"

void parse_args(struct config *cfg, int argc, char **argv);
void usage(const char *pname);

int
main(int argc, char **argv)
{
    struct config cfg = { 0 };

    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    cfg_parse_args(&cfg, argc, argv);

    printf(":: [%s] start monitoring: cmd[%s] threshold[%d] files[",
            get_time(), cfg.cmd, cfg.threshold);

    for (int i = 0; i < cfg.nfiles; ++i) {
        fd_register(&cfg, cfg.files[i]);

        if (i < cfg.nfiles-1) {
            printf("%s ", cfg.files[i]);
        } else {
            printf("%s", cfg.files[i]);
        }
    }

    printf("]\n");

    fd_dispatch(&cfg);
    fd_close(&cfg);

    return EXIT_SUCCESS;
}

