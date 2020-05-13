/*
 *  author: Aleksei Kozadaev (2016)
 */

#include <stdio.h>
#include <stdlib.h>

#include "cfg.h"
#include "cmd.h"
#include "compat.h"
#include "log.h"

void register_files(struct config *cfg);

int
main(int argc, char **argv)
{
    struct command cmd = cmd_make();
    struct config cfg = { 0, .cmd = &cmd };
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    cfg_parse_args(&cfg, argc, argv);
    cfg_print_header(&cfg);

    register_files(&cfg);

    fd_dispatch(&cfg);
    fd_close(&cfg);

    cfg_free(&cfg);

    return EXIT_SUCCESS;
}

void
register_files(struct config *cfg)
{
    for (int i = 0; i < cfg->nfiles; ++i) {
        fd_register(cfg, cfg->files[i]);
    }
}
