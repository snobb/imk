/*
 *  log.c
 *  author: Aleksei Kozadaev (2016)
 */

#include <time.h>

const char *
get_time(void)
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    static char st[16] = { '\0' };

    strftime(st, sizeof(st), "%T", tm);
    return st;
}

