#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include <string.h>
#include <errno.h>

#define LOG_PERROR(m)           \
    fprintf(stderr, ":: [%s] "m": %s\n", get_time(), strerror(errno));

#define LOG_PERROR_VA(m, ...)   \
    fprintf(stderr, ":: [%s] "m": %s\n", get_time(), __VA_ARGS__, strerror(errno));

#define LOG_ERR(m)              \
    fprintf(stderr, ":: [%s] "m"\n", get_time());

#define LOG_ERR_VA(m, ...)      \
    fprintf(stderr, ":: [%s] "m"\n", get_time(), __VA_ARGS__);

#define LOG_INFO(m)             \
    fprintf(stdout, ":: [%s] "m"\n", get_time());

#define LOG_INFO_VA(m, ...)     \
    fprintf(stdout, ":: [%s] "m"\n", get_time(), __VA_ARGS__);

#define LOG_DEBUG(fmt, ...)     \
    fprintf(stderr, ":: [%s] "fmt, get_time(), __VA_ARGS__);

const char *get_time(void);

#endif /* __LOG_H__ */
