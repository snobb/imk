/*
 *  cmd.c
 *  author: Aleksei Kozadaev (2018)
 */

#include <stdlib.h>
#include <errno.h>
#include "log.h"

int
cmd_run(const char *cmd)
{
    int rv = system(cmd);
    if (rv < 0) {
        LOG_PERROR("[===== system =====]");
    } else {
        LOG_INFO_VA("[====== %s (exit code %d) =====]",
                rv == 0 ? "ok" : "fail", rv);
    }

    return rv;
}
