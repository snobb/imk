#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include <string.h>

#define LOG_PERROR(m)           \
        fprintf(stderr, ":: [%s] ERROR: "m": %s\n", get_time(), \
                strerror(errno));

#define LOG_ERR(m)              \
        fprintf(stderr, ":: [%s] ERROR: "m"\n", get_time());

#define LOG_ERR_VA(fmt, ...)    \
        fprintf(stderr, ":: [%s] ERROR: "fmt"\n", get_time(), __VA_ARGS__);

#define LOG_INFO(m)             \
        fprintf(stdout, ":: [%s] "m"\n", get_time());

#define LOG_INFO_VA(fmt, ...)   \
        fprintf(stdout, ":: [%s] "fmt"\n", get_time(), __VA_ARGS__);

#ifdef DEBUG
#define LOG_DEBUG(fmt, ...)     \
        fprintf(stderr, ":: [%s] DEBUG: "fmt"\n", get_time(), __VA_ARGS__);
#else
#define LOG_DEBUG(fmt, ...)
#endif

const char *get_time(void);

#endif /* __LOG_H__ */
