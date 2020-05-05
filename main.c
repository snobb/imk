/*
 *  author: Aleksei Kozadaev (2016)
 */

#include <stdio.h>
#include <stdlib.h>

#include "cfg.h"
#include "cmd.h"
#include "compat.h"
#include "log.h"

int
main(int argc, char **argv)
{
    struct command cmd = {0};
    struct config cfg = {0, .cmd = &cmd};

    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    cfg_parse_args(&cfg, argc, argv);

    printf(":: [%s] start monitoring: cmd[%s] cmd-timeout[%d], threshold[%d] files[",
            get_time(), cfg.cmd->path, cfg.cmd->timeout_ms, cfg.threshold);

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

    cfg_free(&cfg);

    return EXIT_SUCCESS;
}

